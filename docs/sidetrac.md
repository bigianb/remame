#Configuration

The configuration starts with the GAME macro:
```cpp
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
```cpp
    GAME_DRIVER_TRAITS(sidetrac,"Side Trak")                                       
    extern game_driver const GAME_NAME(sidetrac) 
    { 
      GAME_DRIVER_TYPE(sidetrac, exidy_state, MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAV),
      "0",   
      "1979",
      "Exidy",
      [] (machine_config &config, device_t &owner) { downcast<exidy_state &>(owner).sidetrac(config); }, 
      INPUT_PORTS_NAME(sidetrac),                                            
      [] (device_t &owner) { downcast<exidy_state &>(owner).init_sidetrac(); },   
      ROM_NAME(sidetrac),                                                     
      nullptr,                                                            
      nullptr,                                                            
      machine_flags::type(ROT0 | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAV | MACHINE_TYPE_ARCADE)),
      "sidetrac" 
    };
```
game_driver is defined as follows:

```cpp
class game_driver
{
public:
	typedef void (*machine_creator_wrapper)(machine_config &, device_t &);
	typedef void (*driver_init_wrapper)(device_t &);

	device_type                 type;               // static type info for driver class
	const char *                parent;             // if this is a clone, the name of the parent
	const char *                year;               // year the game was released
	const char *                manufacturer;       // manufacturer of the game
	machine_creator_wrapper     machine_creator;    // machine driver tokens
	ioport_constructor          ipt;                // pointer to constructor for input ports
	driver_init_wrapper         driver_init;        // DRIVER_INIT callback
	const tiny_rom_entry *      rom;                // pointer to list of ROMs for the game
	const char *                compatible_with;
	const internal_layout *     default_layout;     // default internally defined layout
	machine_flags::type         flags;              // orientation and other flags
	char                        name[MAX_DRIVER_NAME_CHARS + 1]; // short name of the game
};
```
## Device types

This section looks at the macro
```cpp
    GAME_DRIVER_TYPE(sidetrac, exidy_state, MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAV)
```

The definition is

```cpp
#define GAME_DRIVER_TYPE(NAME, CLASS, FLAGS) \
driver_device_creator< \
		CLASS, \
		(GAME_TRAITS_NAME(NAME)::shortname), \
		(GAME_TRAITS_NAME(NAME)::fullname), \
		(GAME_TRAITS_NAME(NAME)::source), \
		game_driver::unemulated_features(FLAGS), \
		game_driver::imperfect_features(FLAGS)>
```

so, for sidetrac the expansion is as follows:

```cpp
driver_device_creator< 
		exidy_state, 
		(GAME_TRAITS_NAME(sidetrac)::shortname), 
		(GAME_TRAITS_NAME(sidetrac)::fullname), 
		(GAME_TRAITS_NAME(sidetrac)::source), 
		game_driver::unemulated_features(MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAV), 
		game_driver::imperfect_features(MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAV)>
```

where:
```cpp
template <
		typename DriverClass,
		char const *ShortName,
		char const *FullName,
		char const *Source,
		emu::detail::device_feature::type Unemulated,
		emu::detail::device_feature::type Imperfect>
constexpr auto driver_device_creator = &emu::detail::driver_tag_func<DriverClass, ShortName, FullName, Source, Unemulated, Imperfect>;
```

and
```cpp
template <class DriverClass, char const *ShortName, char const *FullName, char const *Source, device_feature::type Unemulated, device_feature::type Imperfect>
auto driver_tag_func() { return driver_tag_struct<DriverClass, ShortName, FullName, Source, Unemulated, Imperfect>{ }; };
```

A device_type is a slight of hand to reference a device_type_impl object.

```cpp
typedef emu::detail::device_type_impl const &device_type;
```

```cpp
class device_type_impl
{
private:
	typedef std::unique_ptr<device_t> (*create_func)(device_type_impl const &type, machine_config const &mconfig, char const *tag, device_t *owner, std::uint32_t clock);

	template <typename DeviceClass>
	static std::unique_ptr<device_t> create_device(device_type_impl const &type, machine_config const &mconfig, char const *tag, device_t *owner, std::uint32_t clock)
	{
		return make_unique_clear<DeviceClass>(mconfig, tag, owner, clock);
	}

	template <typename DriverClass>
	static std::unique_ptr<device_t> create_driver(device_type_impl const &type, machine_config const &mconfig, char const *tag, device_t *owner, std::uint32_t clock)
	{
		return make_unique_clear<DriverClass>(mconfig, type, tag);
	}

	create_func const m_creator;
	std::type_info const &m_type;
	char const *const m_shortname;
	char const *const m_fullname;
	char const *const m_source;
	device_feature::type const m_unemulated_features;
	device_feature::type const m_imperfect_features;

	device_type_impl *m_next;

public:

	template <class DriverClass, char const *ShortName, char const *FullName, char const *Source, device_feature::type Unemulated, device_feature::type Imperfect>
	device_type_impl(driver_tag_struct<DriverClass, ShortName, FullName, Source, Unemulated, Imperfect> (*)())
		: m_creator(&create_driver<DriverClass>)
		, m_type(typeid(DriverClass))
		, m_shortname(ShortName)
		, m_fullname(FullName)
		, m_source(Source)
		, m_unemulated_features(DriverClass::unemulated_features() | Unemulated)
		, m_imperfect_features((DriverClass::imperfect_features() & ~Unemulated) | Imperfect)
		, m_next(nullptr)
	{
	}

	std::type_info const &type() const { return m_type; }
	char const *shortname() const { return m_shortname; }
	char const *fullname() const { return m_fullname; }
	char const *source() const { return m_source; }
	device_feature::type unemulated_features() const { return m_unemulated_features; }
	device_feature::type imperfect_features() const { return m_imperfect_features; }

	std::unique_ptr<device_t> operator()(machine_config const &mconfig, char const *tag, device_t *owner, std::uint32_t clock) const
	{
		return m_creator(*this, mconfig, tag, owner, clock);
	}

	explicit operator bool() const { return bool(m_creator); }

};
```
