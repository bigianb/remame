// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Exidy 6502 hardware

    Games supported:
        * Side Trak

    Known bugs:
        * none at this time


'Universal' Game Board V2 (xxL logic, xxA audio)

Name                 Year  CPU    board/rom numbers

Side Trak            1979  6502   STL, STA

****************************************************************************

    Exidy memory map

    0000-00FF R/W Zero Page RAM
    0100-01FF R/W Stack RAM
    0200-03FF R/W Scratchpad RAM

    4000-43FF R/W Screen RAM
    4800-4FFF R/W Character Generator RAM (except Pepper II and Fax)
    5000       W  Motion Object 1 Horizontal Position Latch (sprite 1 X)
    5040       W  Motion Object 1 Vertical Position Latch   (sprite 1 Y)
    5080       W  Motion Object 2 Horizontal Position Latch (sprite 2 X)
    50C0       W  Motion Object 2 Vertical Position Latch   (sprite 2 Y)
    5100       R  Option Dipswitch Port
                  bit 0  coin 2 (NOT inverted) (must activate together with $5103 bit 5)
                  bit 1-2  bonus
                  bit 3-4  coins per play
                  bit 5-6  lives
                  bit 7  US/UK coins
    5100       W  Motion Objects Image Latch
                  Sprite number  bits 0-3 Sprite #1  4-7 Sprite #2
    5101       R  Control Inputs Port
                  bit 0  start 1
                  bit 1  start 2
                  bit 2  right
                  bit 3  left
                  bit 5  up
                  bit 6  down
                  bit 7  coin 1 (must activate together with $5103 bit 6)
    5101       W  Output Control Latch (not used in PEPPER II upright)
                  bit 7  Enable sprite #1
                  bit 6  Enable sprite #2
    5103       R  Interrupt Condition Latch
                  bit 0  LNG0 - supposedly a language DIP switch
                  bit 1  LNG1 - supposedly a language DIP switch
                  bit 2  different for each game, but generally a collision bit
                  bit 3  TABLE - supposedly a cocktail table DIP switch
                  bit 4  different for each game, but generally a collision bit
                  bit 5  coin 2 (must activate together with $5100 bit 0)
                  bit 6  coin 1 (must activate together with $5101 bit 7)
                  bit 7  L256 - VBlank?
    5213       R  IN2 (Mouse Trap)
                  bit 3  blue button
                  bit 2  free play
                  bit 1  red button
                  bit 0  yellow button
    52XX      R/W Audio/Color Board Communications
    8000-FFF9  R  Program memory space
    FFFA-FFFF  R  Interrupt and Reset Vectors

    Exidy Sound Board:
    0000-07FF R/W RAM (mirrored every 0x7f)
    0800-0FFF R/W 6532 Timer
    1000-17FF R/W 6520 PIA
    1800-1FFF R/W 8253 Timer
    2000-27FF bit 0 Channel 1 Filter 1 enable
              bit 1 Channel 1 Filter 2 enable
              bit 2 Channel 2 Filter 1 enable
              bit 3 Channel 2 Filter 2 enable
              bit 4 Channel 3 Filter 1 enable
              bit 5 Channel 3 Filter 2 enable
    2800-2FFF 6840 Timer
    3000      Bit 0..1 Noise select
    3001      Bit 0..2 Channel 1 Amplitude
    3002      Bit 0..2 Channel 2 Amplitude
    3003      Bit 0..2 Channel 3 Amplitude
    5800-7FFF ROM

    5201    Sound board control
            bit 0 note
            bit 1 upper

    IO:
        A7 = 0: R Communication from sound processor
        A6 = 0: R CVSD Clock State
        A5 = 0: W Busy to sound processor
        A4 = 0: W Data to CVSD

***************************************************************************/


//#include "cpu/m6502/m6502.h"
//#include "machine/6821pia.h"
//#include "audio/exidy.h"
//#include "includes/exidy.h"


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void exidy_state::exidy_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x4000, 0x43ff).mirror(0x0400).ram().share("videoram");
	map(0x5000, 0x5000).mirror(0x003f).writeonly().share("sprite1_xpos");
	map(0x5040, 0x5040).mirror(0x003f).writeonly().share("sprite1_ypos");
	map(0x5080, 0x5080).mirror(0x003f).writeonly().share("sprite2_xpos");
	map(0x50c0, 0x50c0).mirror(0x003f).writeonly().share("sprite2_ypos");
	map(0x5100, 0x5100).mirror(0x00fc).portr("DSW");
	map(0x5100, 0x5100).mirror(0x00fc).writeonly().share("spriteno");
	map(0x5101, 0x5101).mirror(0x00fc).portr("IN0");
	map(0x5101, 0x5101).mirror(0x00fc).writeonly().share("sprite_enable");
	map(0x5103, 0x5103).mirror(0x00fc).r(this, FUNC(exidy_state::exidy_interrupt_r));
	map(0x5210, 0x5212).writeonly().share("color_latch");
	map(0x5213, 0x5213).portr("IN2");
}


void exidy_state::sidetrac_map(address_map &map)
{
	exidy_map(map);
	map(0x0800, 0x3fff).rom();
	map(0x4800, 0x4fff).rom().share("characterram");
	map(0x5200, 0x5200).w(this, FUNC(exidy_state::targ_audio_1_w));
	map(0x5201, 0x5201).w(this, FUNC(exidy_state::spectar_audio_2_w));
	map(0xff00, 0xffff).rom().region("maincpu", 0x3f00);
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/
static void construct_ioport_sidetrac(device_t &owner, ioport_list &portlist, std::string &errorbuf)
{ 
	ioport_configurer configurer(owner, portlist, errorbuf);

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	/* 0x0c same as 0x08 */
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, "Top Score Award" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("INTSOURCE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
}

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/
/*
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1), STEP8(16*8,1), },
	{ STEP16(0,8) },
	8*32
};
*/
/*
static GFXDECODE_START( exidy )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout, 0, 2 )
GFXDECODE_END
*/


MACHINE_CONFIG_START(exidy_state::base)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, EXIDY_CPU_CLOCK)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", exidy_state,  exidy_vblank_interrupt)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", exidy)
	MCFG_PALETTE_ADD("palette", 8)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_RAW_PARAMS(EXIDY_PIXEL_CLOCK, EXIDY_HTOTAL, EXIDY_HBEND, EXIDY_HBSTART, EXIDY_VTOTAL, EXIDY_VBEND, EXIDY_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(exidy_state, screen_update_exidy)
	MCFG_SCREEN_PALETTE("palette")

MACHINE_CONFIG_END


MACHINE_CONFIG_START(exidy_state::sidetrac)
	base(config);

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sidetrac_map)

	/* audio hardware */
	spectar_audio(config);
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( sidetrac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stl8a-1",  0x2800, 0x0800, CRC(e41750ff) SHA1(3868a0d7e34a5118b39b31cff9e4fc839df541ff) )
	ROM_LOAD( "stl7a-2",  0x3000, 0x0800, CRC(57fb28dc) SHA1(6addd633d655d6a56b3e509d18e5f7c0ab2d0fbb) )
	ROM_LOAD( "stl6a-2",  0x3800, 0x0800, CRC(4226d469) SHA1(fd18b732b66082988b01e04adc2b1e5dae410c98) )
	ROM_LOAD( "stl9c-1",  0x4800, 0x0400, CRC(08710a84) SHA1(4bff254a14af7c968656ccc85277d31ab5a8f0c4) ) /* PROM instead of RAM char generator */

	ROM_REGION( 0x0200, "gfx1", 0 )
	ROM_LOAD( "stl11d",   0x0000, 0x0200, CRC(3bd1acc1) SHA1(06f900cb8f56cd4215c5fbf58a852426d390e0c1) )
ROM_END


/*************************************
 *
 *  Driver init
 *
 *************************************/

DRIVER_INIT_MEMBER(exidy_state,sidetrac)
{
	exidy_video_config(0x00, 0x00, false);

	/* hard-coded palette controlled via 8x3 DIP switches on the board */
	m_color_latch[2] = 0xf8;
	m_color_latch[1] = 0xdc;
	m_color_latch[0] = 0xb8;
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/

// "Side Track" on title screen, but cabinet/flyers/documentation clearly indicates otherwise, "Side Trak" it is
//GAME( 1979, sidetrac, 0,       sidetrac, sidetrac, exidy_state, sidetrac, ROT0, "Exidy",   "Side Trak", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

namespace { 
	struct driver_sidetractraits {
        static constexpr char const shortname[] = "sidetrac",
                                    fullname[] = "Side Trak",
                                    source[] = __FILE__;
         };
	constexpr char const    driver_sidetractraits::shortname[],
                            driver_sidetractraits::fullname[],
                            driver_sidetractraits::source[];
} 

extern game_driver const driver_sidetrac
{
  driver_device_creator<
		exidy_state,
		"sidetrac",
		"Side Trak",
		__FILE__,
		game_driver::unemulated_features(MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAV),
		game_driver::imperfect_features(MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAV)>,
  "0",   
  "1979",
  "Exidy",
  // machine_creator_wrapper
  [] (machine_config &config, device_t &owner) { downcast<exidy_state &>(owner).sidetrac(config); },
  construct_ioport_sidetrac,                                           
  [] (device_t &owner) { downcast<exidy_state &>(owner).init_sidetrac(); },   
  ROM_NAME(sidetrac),                                                     
  nullptr,                                                            
  nullptr,                                                            
  machine_flags::type(ROT0 | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAV | MACHINE_TYPE_ARCADE)),
  "sidetrac"
};
