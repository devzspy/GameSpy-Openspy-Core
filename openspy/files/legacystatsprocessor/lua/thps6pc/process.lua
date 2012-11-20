-- Load_BO Load_AU Load_BA Load_BE Load_NO Load_ST Load_SE Load_SE2 Load_TR Load_LA Load_SC Load_PH Load_DJ Load_CA Load_AP
local int_map_crcs = { 706189655, -61328402, -844123056, -893226935, -2036148645, -206867541, -1726262439,  -621173885, 1435058265, 1395431902, 1886975596, -863536089, 203934302,-726366959, -1942829215}
-- nettrickattack netgraffiti netscorechallenge	netcombomambo netslap netking netgoalattack netctf netfirefight netelimiskate
local gametype_crcs = { 818085795, 1580115212, 345515018, -989134896, -103425741, 1861811616, -333443414, 1818227302, -1074579968, 284705398}
local gameid = 1003
function ProcessResults(profileid, snapshotdata, done) 
	local map = find_paramint(snapshotdata, "mapcrc")
	local gametype = find_paramint(snapshotdata,"gametypecrc")
	local mapindex = -1
	for i,v in ipairs(int_map_crcs) do
		if v == map then
			mapindex = i --index 0 is for ranking, etc
		end
	end
	if mapindex == -1 then
		return 0
	end
	if gametype == -333443414 then
		return 0
	end
	--used to detect if people were cheating
	local cheating = find_paramint(snapshotdata,"highscore")
	if cheating > 0 then
		return 0
	end
	local max_players = find_paramint(snapshotdata, "maxplayers")
	max_players = max_players - 1
	local total_players = 0
	for x=0,max_players do
		local tscore = find_paramint(snapshotdata,"score_"..x)
		if tscore > 0 then
			total_players = total_players + 1
		end
	end
	for i=0,max_players do
		local playerscore = find_paramint(snapshotdata,"score_"..i)
		local playercombo = find_paramint(snapshotdata,"combo_"..i)
		local pid = find_paramint(snapshotdata,"pid_"..i)
		local currentName = find_param(snapshotdata,"player_"..i)
		local updateRank = 0
		local pct = 100
		if pid > 0 then
			local currentRating = GetPlayerIntValue(gameid, 2, 0, pid, "THUG2Rating")
			local curHighScore = GetPlayerIntValue(gameid, 2, mapindex, pid, "THUG2HighScore")
			local curHighCombo = GetPlayerIntValue(gameid, 2, mapindex, pid, "THUG2HighCombo")
			local totalMapIncreases = GetPlayerIntValue(gameid, 0, mapindex, pid, "TotalHSIncs")
			local totalMapIncreases_Combo = GetPlayerIntValue(gameid, 0, mapindex, pid, "TotalCIncs")
			if total_players > 0 then
				if totalMapIncreases > 0 then
					pct = pct * (totalMapIncreases/total_players)
				end
			end
			if playerscore > curHighScore then
				SetPlayerIntValue(gameid, 2, mapindex, pid, "THUG2HighScore", playerscore)
				SetPlayerIntValue(gameid, 0, mapindex, pid, "TotalHSIncs", totalMapIncreases+1)
				updateRank = 1
			end
			if playercombo > curHighCombo then
				SetPlayerIntValue(gameid, 2, mapindex, pid, "THUG2HighCombo", playercombo)
				SetPlayerIntValue(gameid, 0, mapindex, pid, "TotalCIncs", totalMapIncreases_Combo+1)
--				updateRank = 1
			end
			if updateRank == 1 then
				currentRating = currentRating + pct
				if currentRating > 5000 then
					currentRating = 5000
				end
				SetPlayerIntValue(gameid, 2, 0, pid, "THUG2Rating", currentRating)
			end
			SetPlayerStringValue(gameid, 2, 0, pid, "PlayerName", currentName)
		end
	end
end
