
The configuration starts with the GAME macro:
```C++
  GAME( 1979,         // year
        sidetrac,     // name
        0,            // parent
        sidetrac,     // machine
        sidetrac,     // input
        exidy_state,  // class
        sidetrac,     // init
        ROT0,         // monitor
        "Exidy",      // company
        "Side Trak",  // fullname
        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAV  // flags
        )
```
This expands to:
```C++
    GAME_DRIVER_TRAITS(sidetrac,"Side Trak")                                       
    extern game_driver const GAME_NAME(sidetrac) 
    { 
      GAME_DRIVER_TYPE(sidetrac, exidy_state, MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAV),
      #0,   
      #1979,
      "Exidy",
      [] (machine_config &config, device_t &owner) { downcast<exidy_state &>(owner).sidetrac(config); }, 
      INPUT_PORTS_NAME(sidetrac),                                            
      [] (device_t &owner) { downcast<exidy_state &>(owner).init_sidetrac(); },   
      ROM_NAME(sidetrac),                                                     
      nullptr,                                                            
      nullptr,                                                            
      machine_flags::type(ROT0 | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAV | MACHINE_TYPE_ARCADE)),
      #sidetrac 
    };
```