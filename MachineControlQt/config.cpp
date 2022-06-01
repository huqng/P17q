#include "stdafx.h"
#include "config.h"

mINI::INIStructure iniCfg; 
mINI::INIStructure iniCoord;
mINI::INIStructure iniIO;
mINI::INIStructure iniText;

void LoadIniConfig()
{
	mINI::INIFile fileCfg("./Config/Config.ini");
	fileCfg.read(iniCfg);
}

void LoadIniCoord()
{
	mINI::INIFile fileCoord("./Config/Position.pt");
	fileCoord.read(iniCoord);
}
	
void LoadIniIO()
{
	mINI::INIFile fileCfg("./Config/IO.ini");
	fileCfg.read(iniIO);
}

void LoadIniText()
{
	mINI::INIFile fileCfg("./Config/Text.ini");
	fileCfg.read(iniText);
}
