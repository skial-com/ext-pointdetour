#include "extension.h"
#include "CDetour/detours.h"

/**
 * Globals
 */
PSCDetour g_Interface;
SMEXT_LINK(&g_Interface);

IGameConfig *g_pGameConf = NULL;
IForward *g_pPSCommandForward = NULL;
IForward *g_pPCCommandForward = NULL;

CDetour *detourPointServerCommand = NULL;
CDetour *detourPointClientCommand = NULL;

DETOUR_DECL_MEMBER1(PointServerCommand, void, inputdata_t &, inputdata)
{
	if (!inputdata.value.pszValue || !inputdata.value.pszValue[0])
	{
		return;
	}
	
	cell_t cellPawnResult = Pl_Continue;
	
	g_pPSCommandForward->PushString(inputdata.value.pszValue);
	g_pPSCommandForward->Execute(&cellPawnResult);
	
	if (cellPawnResult != Pl_Continue)
	{
		return;
	}
	
	DETOUR_MEMBER_CALL(PointServerCommand)(inputdata);
}

DETOUR_DECL_MEMBER1(PointClientCommand, void, inputdata_t &, inputdata)
{
	if (!inputdata.value.pszValue[0])
	{
		return;
	}
	
	CBaseEntity *pEnt = (CBaseEntity *)(inputdata.pActivator);
	
	if (!pEnt)
	{
		return;
	}
	
	int iClientIndex = gamehelpers->EntityToBCompatRef(pEnt);
	
	cell_t cellPawnResult = Pl_Continue;
	
	g_pPCCommandForward->PushCell(iClientIndex);
	g_pPCCommandForward->PushString(inputdata.value.pszValue);
	g_pPCCommandForward->Execute(&cellPawnResult);
	
	if (cellPawnResult != Pl_Continue)
	{
		return;
	}
	
	DETOUR_MEMBER_CALL(PointClientCommand)(inputdata);
}

bool PSCDetour::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	char conf_error[255] = "";
	if(!gameconfs->LoadGameConfigFile("PSCDetour", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if(conf_error[0])
		{
			snprintf(error, maxlength, "Could not read PSCDetour.txt: %s", conf_error);
		}
		
		return false;
	}

	g_pPSCommandForward = forwards->CreateForward("PointServerCommandForward", ET_Hook, 1, NULL, Param_String);
	g_pPCCommandForward = forwards->CreateForward("PointClientCommandForward", ET_Hook, 2, NULL, Param_Cell, Param_String);
	
	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);

	detourPointServerCommand = DETOUR_CREATE_MEMBER(PointServerCommand, "PointServerCommand");

	if (detourPointServerCommand == NULL)
	{
		return false;
	}
	
	detourPointServerCommand->EnableDetour();
	
	detourPointClientCommand = DETOUR_CREATE_MEMBER(PointClientCommand, "PointClientCommand");
	
	if (detourPointClientCommand == NULL)
	{
		return false;
	}
	
	detourPointClientCommand->EnableDetour();
	return true;
}

void PSCDetour::SDK_OnAllLoaded()
{

}

void PSCDetour::SDK_OnUnload()
{
	if (g_pPSCommandForward != NULL)
	{
		forwards->ReleaseForward(g_pPSCommandForward);
	}
	
	if (g_pPCCommandForward != NULL)
	{
		forwards->ReleaseForward(g_pPCCommandForward);
	}
	
	if (detourPointServerCommand != NULL)
	{
		detourPointServerCommand->Destroy();
		detourPointServerCommand = NULL;
	}

	if (detourPointClientCommand != NULL)
	{
		detourPointClientCommand->Destroy();
		detourPointClientCommand = NULL;
	}
	
	gameconfs->CloseGameConfigFile(g_pGameConf);
}

bool PSCDetour::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	return true;
}
