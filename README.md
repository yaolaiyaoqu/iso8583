# iso8583
iso8583 for lua

make

iso8583.so can be used in lua 

copy iso8583.so to lua cpath

example：

local iso8583 = require("iso8583")

local iso8583_str = "0100202000000080801B350000000453313331303532303231353600183030303030303030303030303030303030300002009A002031313130303031393030303039363330303033353242443430443338"

local function to_bin(str)
    return ({str:gsub("..", function(x) return string.char(tonumber(x, 16)) end)})[1]
end

local function to_hex(str)
    return ({str:gsub(".", function(c) return string.format("%02X", c:byte(1)) end)})[1]
end

print(iso8583_str)

iso8583_str = to_bin(iso8583_str)

local parser, err = iso8583.new({
        [40] = { size = 999, type = iso8583.LLLVAR, align = iso8583.L, pad = 'F', compress = iso8583.U },
        [43] = { size = 18,  type = iso8583.FIX,    align = iso8583.L, pad = ' ', compress = iso8583.U },
        [44] = { size = 99,  type = iso8583.LLVAR,  align = iso8583.R, pad = '0', compress = iso8583.U },
        [48] = { size = 999, type = iso8583.LLLVAR, align = iso8583.L, pad = 'F', compress = iso8583.Z },
        [60] = { size = 999, type = iso8583.LLLVAR, align = iso8583.L, pad = 'F', compress = iso8583.U },
        [61] = { size = 999, type = iso8583.LLLVAR, align = iso8583.L, pad = 'F', compress = iso8583.U },
})

local msg, err = parser:Unpack(iso8583_str)

-- if success msg is a parsed table, and err is the length of the iso8583 message

for k, v in pairs(msg) do
        print(k, v)
end

local packed_str, err = parser:Pack(msg)

if not packed_str then
        print(err)
        return
end

local packed_str_hex = to_hex(packed_str)

print(packed_str_hex)
