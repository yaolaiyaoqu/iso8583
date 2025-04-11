# luaiso8583

This is a small lua module. Can unpack and package iso8583 messages.

## Table of Contents

* [Install](#install)
* [API](#api)
    * [Constant](#constant)
    * [new](#new)
    * [Pack](#pack)
    * [Unpack](#unpack)

## Install

```
$ git clone git@github.com:yaolaiyaoqu/iso8583.git
$ cd iso8583
$ make
```

To specify the Lua version, use -e LUA_VERSION=xxx. The default Lua version is 5.1.

```
$ git clone git@github.com:yaolaiyaoqu/iso8583.git
$ cd iso8583
$ make -e LUA_VERSION=5.4
```

[Back to TOC](#table-of-contents)

## API

`local iso8583 = require('iso8583');`

[Back to TOC](#table-of-contents)

### Constant

There are following constant in the module:

* iso8583.FIX
* iso8583.LLVAR
* iso8583.LLLVAR
* iso8583.RIGHT
* iso8583.R
* iso8583.LEFT
* iso8583.L
* iso8583.U
* iso8583.UNZIP
* iso8583.Z
* iso8583.ZIP
* iso8583.LLVAR_U
* iso8583.LLLVAR_U

[Back to TOC](#table-of-contents)

### new

`syntax: obj, err = iso8583.new(fields)`

Creates an iso8583 object. In case of failures, returns `nil` and a string describing the error.

The fields is an array of lua tables holding the following keys:

* `size`

    The size of the field.

* `type`

    The type of the field, it must be FIX, LLVAR, LLLVAR, LLVAR_U, LLLVAR_U. U means ASCII prefix.

* `align`

    Align of the field, it must be R, L.

* `pad`

    Pad character of the field.

* `compress`

    U or Z, Z means BCD data, U means BIN data.

The default definition of fields is in [iso8583.c](iso8583.c#L57).

[Back to TOC](#table-of-contents)

### Pack

`syntax: message, err = obj:Pack(data)`

Pack iso8583 data to message. In case of failures, returns `nil` and a string describing the error.

[Back to TOC](#table-of-contents)

### Unpack

`syntax: data, err = obj:Unpack(message)`

Unpack message to iso8583 data. In case of failures, returns `nil` and a string describing the error.

[Back to TOC](#table-of-contents)

## License

