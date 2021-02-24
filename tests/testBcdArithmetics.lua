bcd = require "bcd"
lapp = require( 'pl.lapp')

local args = lapp( [[
Test program for Lua bcd module
	-h,--help     Print usage
	-V,--verbose  Verbose output
]])
if args.help then
	print( "Usage: testBcdArithmetics.lua [-h][-V]")
	exit( 0)
end
local verbose = args.verbose

function checkResult( testname, output, expected)
	if not output == expected then
		io.stderr:write( "OUPUT:  " .. tostring(output) .. "\n")
		io.stderr:write( "EXPECT: " .. tostring(expected) .. "\n")
		error( "Test " .. testname .. " failed")
	end
end

function test_add( arg1, arg2, expect)
	local result = bcd.int( arg1) + arg2
	if verbose then
		print( "Test " .. arg1 .. " + " .. arg2 .. "\n = " .. tostring(result))
	end
	checkResult( "add", result, expect)
end

function test_sub( arg1, arg2, expect)
	local result = bcd.int( arg1) - arg2
	if verbose then
		print( "Test " .. arg1 .. " - " .. arg2 .. "\n = " .. tostring(result))
	end
	checkResult( "sub", result, expect)
end

function test_unm( arg, expect)
	local result = - bcd.int( arg)
	if verbose then
		print( "Test -" .. arg .. "\n = " .. tostring(result))
	end
	checkResult( "sub", result, expect)
end

function test_mul( arg1, arg2, expect)
	local result = bcd.int( arg1) * arg2
	if verbose then
		print( "Test " .. arg1 .. " * " .. arg2 .. "\n = " .. tostring(result))
	end
	checkResult( "mul", result, expect)
end

function test_div( arg1, arg2, expect)
	local result = (bcd.int( arg1) / arg2)
	if verbose then
		print( "Test " .. arg1 .. " / " .. arg2 .. "\n = " .. tostring(result))
	end
	checkResult( "div result", result, expect)
end

function test_div2( arg1, arg2, expect, expect_remainder)
	local result,rm = bcd.int( arg1):div( arg2)
	if verbose then
		print( "Test " .. arg1 .. ":div( " .. arg2 .. ")\n = " .. tostring(result) .. ", " ..  tostring(rm))
	end
	checkResult( "div result", result, expect)
	checkResult( "div remainder", rm, expect_remainder)
end

function test_mod( arg1, arg2, expect)
	local result = bcd.int( arg1) % arg2
	if verbose then
		print( "Test " .. arg1 .. " % " .. arg2 .. "\n = " .. tostring(result))
	end
	checkResult( "mod", result, expect)
end

function test_pow( arg1, arg2, expect)
	local result = bcd.int( arg1) ^ arg2
	if verbose then
		print( "Test " .. arg1 .. " ^ " .. arg2 .. "\n = " .. tostring(result))
	end
	checkResult( "mod", result, expect)
end

local bits64 = bcd.bits(64)

function test_bitwise_and( arg1, arg2, expect)
	local result = bcd.int( arg1):bit_and( arg2, bits64)
	if verbose then
		print( "Test bcd.int( " .. arg1 .. "):bit_and( " .. arg2 .. ", bits64)\n = " .. tostring(result))
	end
	checkResult( "mod", result, expect)
end

function test_bitwise_or( arg1, arg2, expect)
	local result = bcd.int( arg1):bit_or( arg2, bits64)
	if verbose then
		print( "Test bcd.int( " .. arg1 .. "):bit_or( " .. arg2 .. ", bits64)\n = " .. tostring(result))
	end
	checkResult( "mod", result, expect)
end

function test_bitwise_xor( arg1, arg2, expect)
	local result = bcd.int( arg1):bit_xor( arg2, bits64)
	if verbose then
		print( "Test bcd.int( " .. arg1 .. "):bit_xor( " .. arg2 .. ", bits64)\n = " .. tostring(result))
	end
	checkResult( "mod", result, expect)
end

function test_bitwise_not( arg, expect)
	local result = bcd.int( arg):bit_not( bits64)
	if verbose then
		print( "Test bcd.int( " .. arg .. "):bit_not( bits64)\n = " .. tostring(result))
	end
	checkResult( "mod", result, expect)
end

test_add( "1091274089731205741574315105408501238459018244", "09837450983259878234932079584098479356329382873490537340570384",
		"9837450983259879326206169315304220930644488281991775799588628")
test_sub( "9082873327498632874670832947632592380417269304829645738789127340936479873287459875943",
		"902346320843927649082367469728197367489641319785207397461923742976453290432",
		"9082873326596286553826905298550224910689071937340004419003919943474556130311006585511")
test_unm( "902342398740329759432765872436574836529034592435", "-902342398740329759432765872436574836529034592435")
test_div( "987634312046372657243165894732984627528652743256289", "489234689743276590",
		"2018733202595729114985207456503685")
test_div2( "987634312046372657243165894732984627528652743256289", "489234689743276590",
		"2018733202595729114985207456503685", "370487055434022139")
test_mul( "0928371943675932874568502547967845730265254230214350790843750295746572438246723875240396738754528068705942",
		"39487234590423085763409320895769851928347032465784012647436754821376",
		"36658840727098609307697432257185681689428878561927181333141886403981219498661407725702082804606976599086870750054575860734996213845434704579807433483080295407315953679816192")
test_mod( "30942103589712319893284128990876865428891253462134879327434651029345238746374832478534895727852664945893",
		"1209487632765213498032",
		"809309430900907004341")
test_pow( "3", "3", "27" )
test_pow( "3432", "324",
		"32909285492191702601486641617030895261336571028125928148482029183417" ..
		"76420408529430156926478229641243070090985829566723127262384566085153" ..
		"20234589365626978437980945010363224475473175030528410432816326329398" ..
		"12647845865836205045786197513427947183909883352884996918471465463477" ..
		"34953208538654514020014503823989643975086667300679562268253181486009" ..
		"61113848818963039772276480488371551839601879815374598113417189255975" ..
		"42904250797839254558677143935744794402954737409971931227640991852983" ..
		"17508495079438255976014396529128824515258852609979051915667244610041" ..
		"02121417934675750023520820007601438675694191928678950241018510193338" ..
		"08319986767366426833549542651613618748317056874719133697413020829840" ..
		"53327849745997691896843865651770014399902637249770202207559344211634" ..
		"18841475508235034280002313949585712309341433111742655178979958588804" ..
		"79251193965703189595592930304032198596345449684502715356550205805579" ..
		"20582460076794455890444600770827202305478154276952881667660299711633" ..
		"36466383463798658047987437720922762342224029720601714890158421689199" ..
		"33691116060472959502025027634958237190003423128920297045827782588699" ..
		"3090255435128824256023942282058827464021476042241921253376" )

test_bitwise_and( "3", "1", "1" )
test_bitwise_and( "29341730247", "918273", "393473" )
test_bitwise_or( "434254654", "983476324", "1006549886" )
test_bitwise_xor( "434254654", "983476324", "595368794" )
test_bitwise_not( "434254654", bcd.int( "434254654"):bit_xor( bcd.int(2) ^ 64 - 1, bits64) )

checkResult( "BCD from float", tostring(bcd.int(7.23)), "7")
if verbose then print( "Test BCD from float 7.23 = 7") end

print( "OK")

