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
	if not output:__eq(expected) then
		io.stderr:write( "OUPUT:  " .. tostring(output) .. "\n")
		io.stderr:write( "EXPECT: " .. expected .. "\n")
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

print( "OK")
