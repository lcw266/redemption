1. [Add/Modify variable](#addmodify-variable)
    1. [MemberInfo.value](#memberinfovalue)
        1. [Special values](#special-values)
        2. [Special types](#special-types)
        3. [Conn Policy Value](#conn-policy-value)
    2. [MemberInfo.spec](#memberinfospec)
        1. [SesmanInfo](#sesmaninfo)
            1. [ResetBackToSelector](#resetbacktoselector)
            2. [Loggable](#loggable)
        2. [Ini only](#ini-only)
        3. [Global Spec](#global-spec)
            1. [Attributes](#attributes)
        4. [Conn Policy](#conn-policy)
            1. [Attributes](#attributes-1)
2. [Add/Modify enumeration type](#addmodify-enumeration-type)
3. [Build](#build)


# Add/Modify variable

Edit `configs_specs/configs/specs/config_spec.hpp`

Edit `configs_specs/configs/specs/config_type.hpp` to add a new enumeration type.


## MemberInfo.value

- `enum_as_string(value, conn_policy_value...)`: Initialize with an enum and use string parser for `.spec`
- `from_enum(value, conn_policy_value...)`: Initialize with an enum and use integer parser for `.spec` file
- `value<T>(value, conn_policy_value...)`: Initialize with a type `T`
- `value<T>()`: equivalent to `value<T>(T{})`

### Special values

- `cpp::expr{"expr as string"}`
- `CPP_EXPR(expression)` (for macro: `CPP_EXPR(MACRO_NAME)`). Equivalent to `cpp::expr{#expression"}`

### Special types

- `types::u8` for `uint8_t`
- `types::u16` for `uint16_t`
- `types::u32` for `uint32_t`
- `types::u64` for `uint64_t`
- `types::i8` for `int8_t`
- `types::i16` for `int16_t`
- `types::i32` for `int32_t`
- `types::i64` for `int64_t`
- `types::int_` for `int`
- `types::unsigned_` for `unsigned`
- `types::list<type>`: comma-separated values (cf: `0, 3, 4`)
- `types::ip_string`
- `types::dirpath`: always `/` terminated. Note: use `std::string` for file path type.
- `types::range<type, min, max>`
- `types::fixed_string<n>`: size without zero-terminal.
- `types::fixed_binary<n>`: size without zero-terminal.
- `types::megabytes<type>`: value in magabytes
- `types::rgb`: a color


### Conn Policy Value

Configures `sesman` to send a different default depending on the connection policy selected. Used in conjunction with `MemberInfo.spec u003d connpolicy(...)`.

- `rdp_policy_value(value)`: Default value for rdp.spec
- `vnc_policy_value(value)`: Default value for vnc.spec
- `jh_policy_value(value)`: Default value for jh.spec

- `rdp/vnc/jh_policy_value(value).always()`: The value is not configurable through `.spec` and is always initialized with the specified value.


## MemberInfo.spec

### SesmanInfo

ACL (Access Control List) refers to `sesman` or `passthrough.py` which have the ability to send or receive configuration options.

- `no_acl`
- `proxy_to_acl(ResetBackToSelector)`
- `acl_to_proxy(ResetBackToSelector, Loggable)`
- `acl_rw(ResetBackToSelector, Loggable)`: proxy_to_acl + acl_to_proxy

#### ResetBackToSelector

Indicates whether ACLs should send value when you return to the internal pages before the connection to target.

```cpp
auto reset_back_to_selector = ResetBackToSelector::Yes;
auto no_reset_back_to_selector = ResetBackToSelector::No;
```

#### Loggable

Indicates how to display values when debug logging is enabled.

```cpp
auto L = Loggable::Yes;
auto NL = Loggable::No;
auto VNL = Loggable::OnlyWhenContainsPasswordString;
```

### Ini only

- `spec::ini_only(SesmanInfo)`

### Global Spec

Correspondent au fichier `.spec` utilisé par wallix bastion pour afficher une GUI.

- `spec::global_spec(SesmanInfo, attributes = {})`: global spec file generated with `rdpproxy --print-spec`
- `spec::external(attributes = {})`: Les valeurs ne sont pas utilisé par le proxy, mais par d'autre programme qui vont lire le même fichier ini.

#### Attributes

- `spec::hex`: preferably display as hex
- `spec::advanced`: mark as advanced option
- `spec::iptables`: reconfigure iptable
- `spec::password`: the field is a password
- `spec::acl_only`: implicit with `spec::external`
- `spec::restart_service`: the service will be restarted
- `spec::logged`: logged the modifications
- `spec::image(path)`: the field is an image

### Conn Policy

Correspondent au fichier `.spec` utilisé par wallix bastion pour afficher une GUI. En dehors d'un bastion, cela correspond à `acl_to_proxy(ResetBackToSelector::No, ...)`.

- `spec::connpolicy(vnc | rdp_and_jh | rdp_without_jh, Loggable, attributes = {})`

#### Attributes

- `spec::hex`: preferably display as hex
- `spec::advanced`: mark as advanced option
- `spec::password`: the field is a password
- `spec::acl_only`: the field is sent to the proxy only for logging



# Add/Modify enumeration type

Edit `configs_specs/configs/specs/config_type.hpp`

```cpp
// enum { a = 1, b = 2, c = 4, ... }
e.enumeration_flags(enum_name[, enum_desc[, enum_info]])
    .value(value_name[, value_desc])[.alias(alias_name)...][.exclude()...]
    ...

// enum { a, b, c, ... }
e.enumeration_list(enum_name[, enum_desc[, enum_info]])
    .value(value_name[, value_desc])[.alias(alias_name)...][.exclude()...]
    ...

// enum { a = v1, b = v2, c = v3, ... }
e.enumeration_set(enum_name[, enum_desc[, enum_info]])
    .value(value_name, uint_val[, value_desc])[.alias(alias_name)...]
    ...

// enum_info: description after values
```


# Build

```bash
bjam
```

or

```bash
bjam generate_cpp_enumeration
bjam generate_config_spec
```
