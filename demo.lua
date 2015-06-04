iso8583 = require("iso8583")

--iso8583_str = "01002020000000808019350000000509323039353036343731353600183030303030303030303030303030303030300002015b3434424643413144"
iso8583_str = "0100202000000080801B350000000453313331303532303231353600183030303030303030303030303030303030300002009A002031313130303031393030303039363330303033353242443430443338"

function to_bin(str)
    return ({str:gsub("..", function(x) return string.char(tonumber(x, 16)) end)})[1]
end

function to_hex(str)
    return ({str:gsub(".", function(c) return string.format("%02X", c:byte(1)) end)})[1]
end

print(iso8583_str)
iso8583_str = to_bin(iso8583_str)

msg,err = iso8583.new({
	[40] = {size = 999, type = iso8583.LLLVAR, align = iso8583.L, pad = 'F', compress = iso8583.U},
	[43] = {size = 18,  type = iso8583.FIX,    align = iso8583.L, pad = ' ', compress = iso8583.U},
	[44] = {size = 99,  type = iso8583.LLVAR,  align = iso8583.R, pad = '0', compress = iso8583.U},
	[48] = {size = 999, type = iso8583.LLLVAR, align = iso8583.L, pad = 'F', compress = iso8583.Z},
	[60] = {size = 999, type = iso8583.LLLVAR, align = iso8583.L, pad = 'F', compress = iso8583.U},
	[61] = {size = 999, type = iso8583.LLLVAR, align = iso8583.L, pad = 'F', compress = iso8583.U},
})

a, err = msg:Unpack(iso8583_str)

for k, v in pairs(a) do
	print(k .. ":" .. v)
end

packed_str, err = msg:Pack(a)

packed_str_hex = to_hex(packed_str)

print(packed_str_hex)
