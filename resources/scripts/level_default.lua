--
-- Edit as you like, but keep the order! All methods
-- have been placed in the appropriate order. You may
-- risk crashing the application by moving them around
--

---------------------------------------------------

InitPlayers(4)

---------------------------------------------------

LoadHeightmap("das_bana.pgm")
SetMaximumBuildingAltitude(-0.0005)
SetWaterColor(0, 0, 0)

---------------------------------------------------

map_texture = LoadTerrainTexture("terrain3.png")
PrepareGUI(map_texture)
FreeSurface(map_texture)

---------------------------------------------------

InitSkybox(30, 9)

---------------------------------------------------

SetHourLength(30.0)
SetDayLength(24)

---------------------------------------------------
day = AllocEnvironmentalCondition()

SetHours(day, 6, 18)
SetType(day, "day")
SetMusicList(day, "musicDay")
SetSkybox(day, "day02")
SetSunPos(day, 1024.0, 1024.0, 1024.0, 1.0)
SetDiffuse(day, 1.0, 1.0, 1.0, 1.0)
SetAmbient(day, 1.0, 1.0, 1.0, 1.0)
SetFogParams(day, 12.0, 14.0, 0.15)
SetFogColor(day, -1.0, -1.0, -1.0, -1.0)

---------------------------------------------------
night = AllocEnvironmentalCondition()

SetHours(night, 18, 6)
SetType(night, "night")
SetMusicList(night, "musicNight")
SetSkybox(night, "night01")
SetSunPos(night, 1024.0, 1024.0, 1024.0, 1.0)
SetDiffuse(night, 0.2, 0.2, 0.2, 1.0)
SetAmbient(night, 0.2, 0.2, 0.2, 1.0)
SetFogParams(night, 8.5, 12.5, 0.4)
SetFogColor(night, -1.0, -1.0, -1.0, -1.0)

AddEnvironmentalCondition(day)
AddEnvironmentalCondition(night)

if ValidateEnvironmentalConditions() == false then
	Output ("Environmental condition validation failed!!!")
end

---------------------------------------------------

SetCurrentHour(6)

---------------------------------------------------

cur_player = GetPlayerByIndex(0) -- GAIA
CreateUnit(GetUnitTypeFromString("MediumLightTower"), cur_player, 107.5,  117.5, 0)
CreateUnit(GetUnitTypeFromString("SolarPanel"),       cur_player, 103.5,  124.5, 0)
CreateUnit(GetUnitTypeFromString("SolarPanel"),       cur_player, 100.5,  111.5, 0)
CreateUnit(GetUnitTypeFromString("SolarPanel"),       cur_player, 112.5,  117.5, 0)

cur_player = GetPlayerByIndex(1)
CreateUnit(GetUnitTypeFromString("MainBuilding"), cur_player, 39.5,  40.5, 0)
CreateUnit(GetUnitTypeFromString("Builder"),      cur_player, 43.5,  50.5, 0)
CreateUnit(GetUnitTypeFromString("SolarPanel"),   cur_player, 46.5,  40.5, 0)
CreateUnit(GetUnitTypeFromString("SolarPanel"),   cur_player, 31.5,  39.5, 0)
CreateUnit(GetUnitTypeFromString("LargeTank"),    cur_player, 37.5,  51.5, 0)

cur_player = GetPlayerByIndex(2)
CreateUnit(GetUnitTypeFromString("MainBuilding"), cur_player, 221.5,  225.5, 0)
CreateUnit(GetUnitTypeFromString("Builder"),      cur_player, 226.5,  226.5, 0)
CreateUnit(GetUnitTypeFromString("SolarPanel"),   cur_player, 216.5,  235.5, 0)
CreateUnit(GetUnitTypeFromString("SolarPanel"),   cur_player, 230.5,  228.5, 0)

cur_player = GetPlayerByIndex(3)
CreateUnit(GetUnitTypeFromString("MainBuilding"), cur_player, 223.5,  47.5, 0)
CreateUnit(GetUnitTypeFromString("Builder"),      cur_player, 222.5,  54.5, 0)
CreateUnit(GetUnitTypeFromString("SolarPanel"),   cur_player, 215.5,  55.5, 0)
CreateUnit(GetUnitTypeFromString("SolarPanel"),   cur_player, 214.5,  44.5, 0)

cur_player = GetPlayerByIndex(4)
CreateUnit(GetUnitTypeFromString("MainBuilding"), cur_player, 34.5,  224.5, 0)
CreateUnit(GetUnitTypeFromString("Builder"),      cur_player, 40.5,  223.5, 0)
CreateUnit(GetUnitTypeFromString("SolarPanel"),   cur_player, 42.5,  229.5, 0)
CreateUnit(GetUnitTypeFromString("SolarPanel"),   cur_player, 28.5,  222.5, 0)