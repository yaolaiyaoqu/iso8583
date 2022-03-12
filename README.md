# Title

This is a small lua module. Can unpack and package iso8583 messages.

## Install

```
$ git clone git@github.com:yaolaiyaoqu/iso8583.git
$ cd iso8583
$ make
```

## API

`local iso8583 = require('iso8583');`

### iso8583.new(fields)

Create an iso8583 object.

```lua
local object, err = iso8583.new(fields)
```

### object:Pack(datas)

Pack iso8583 datas to message.

```lua
local message, err = object:Pack(datas)
```

### object:Unpack(message)

Unpack message to iso8583 datas.

```lua
local datas, err = object:Unpack(message)
```

## License

