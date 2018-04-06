#include "x64dbg_plugin.h"

// function prototypes


// Variables
#define sztest_plugInfo "test_plug plugin by fearless 2016 - www.LetTheLight.in\n\nFeatures & Usage:\n" 


// Plugin SDK required variables
#define plugin_name "x64dbg_plugin" // rename to your plugins name 
#define plugin_version 1

// GLOBAL Plugin SDK variables
int pluginHandle;
HWND hwndDlg;
int hMenu;
int hMenuDisasm;
int hMenuDump;
int hMenuStack;
CBPLUGINFORMATFUNCTION ddd;


/*====================================================================================
  Main entry function for a DLL file  - required.
--------------------------------------------------------------------------------------*/
extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return TRUE;
}


/*====================================================================================
  pluginit - Called by debugger when plugin.dp32 is loaded - needs to be EXPORTED
  
  Arguments: initStruct - a pointer to a PLUG_INITSTRUCT structure

  Notes:     you must fill in the pluginVersion, sdkVersion and pluginName members. 
             The pluginHandle is obtained from the same structure - it may be needed in
             other function calls.
 
             you can call your own setup routine from within this function to setup 
             menus and commands, and pass the initStruct parameter to this function.
 
--------------------------------------------------------------------------------------*/

bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
	_plugin_registerformatfunction(pluginHandle, "dump", [](char* dest, size_t destCount, int argc, char* argv[], duint value, void* userdata)
	{
		unsigned char *buff;
		char *string;
		int cnt = 1, i = 0;
		char mode,*ptr;

		//_plugin_logprintf("0x%08X, %d, %d, %08X, 0x%08X\n", dest, destCount, argc, value, &userdata);
		
		if (!DbgMemIsValidReadPtr(value))
			return FORMAT_ERROR;
		if (argc > 3)
			return FORMAT_ERROR;
		if (argc == 3){
			mode = argv[1][0];
			cnt = atoi(argv[2]);
		}else {
			mode='b';
			cnt = atoi(argv[1]);
		}
		if ((cnt <= 0)){
			return FORMAT_ERROR;
		}
		_plugin_logprint("\n");
		i=0;
		switch(mode){
#ifdef _WIN64
			case 'q':
				if (cnt*8 >= (__int64)destCount)return FORMAT_BUFFER_TOO_SMALL;
				buff = (unsigned char *)BridgeAlloc(destCount);
				string = (char *)BridgeAlloc(destCount*16);
				DbgMemRead(value, buff, cnt*8);
				ptr = string;
				while (cnt--){
					sprintf(ptr, "%016llX", _byteswap_uint64(*(unsigned __int64 *)(&buff[i++*8])));
					ptr += 16;
					//if(!(i % 8)) *ptr++ = '\n';
				}
				break;
#endif
			case 'd':
				if (cnt*4 >= (int)destCount)return FORMAT_BUFFER_TOO_SMALL;
				buff = (unsigned char *)BridgeAlloc(destCount);
				string = (char *)BridgeAlloc(destCount*8);
				DbgMemRead(value, buff, cnt*4);
				ptr = string;
				while (cnt--){
					sprintf(ptr, "%08X", _byteswap_ulong(*(unsigned int *)(&buff[i++*4])));
					ptr += 8;
					//if(!(i % 8)) *ptr++ = '\n';
				}
				break;
			default:
				if (cnt*2 >= (int)destCount)return FORMAT_BUFFER_TOO_SMALL;
				buff = (unsigned char *)BridgeAlloc(destCount);
				string = (char *)BridgeAlloc(destCount*2);
				DbgMemRead(value, buff, cnt);
				ptr = string;
				while (cnt--){
					sprintf(ptr, "%02X", (unsigned char *)buff[i++]);
					ptr +=2;
					//if(!(i % 16)) *ptr++ = '\n';
				}
				break;
		}
		*ptr++=0;
		if (strlen(string)>= destCount)
			return FORMAT_BUFFER_TOO_SMALL;
		strcpy_s(dest, destCount, string);
		BridgeFree(buff);
		BridgeFree(string);
		return FORMAT_SUCCESS;
	}, nullptr);
	return true; //Return false to cancel loading the plugin.
}



DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct)
{
    initStruct->pluginVersion = plugin_version;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strcpy(initStruct->pluginName, plugin_name);
    pluginHandle = initStruct->pluginHandle;

	// place any additional initialization code here

		return pluginInit(initStruct);
}


/*====================================================================================
  plugstop - Called by debugger when the plugin.dp32 is unloaded - needs to be EXPORTED
 
  Arguments: none
  
  Notes:     perform cleanup operations here, clearing menus and other housekeeping
 
--------------------------------------------------------------------------------------*/
DLL_EXPORT bool plugstop()
{
    _plugin_menuclear(hMenu);

	// place any cleanup code here
	
    return true;
}


/*====================================================================================
  plugsetup - Called by debugger to initialize your plugins setup - needs to be EXPORTED
 
  Arguments: setupStruct - a pointer to a PLUG_SETUPSTRUCT structure
  
  Notes:     setupStruct contains useful handles for use within x64_dbg, mainly Qt 
             menu handles (which are not supported with win32 api) and the main window
             handle with this information you can add your own menus and menu items 
             to an existing menu, or one of the predefined supported right click 
             context menus: hMenuDisam, hMenuDump & hMenuStack
             
             plugsetup is called after pluginit. 
--------------------------------------------------------------------------------------*/
DLL_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct)
{
    hwndDlg = setupStruct->hwndDlg;
    hMenu = setupStruct->hMenu;
    hMenuDisasm = setupStruct->hMenuDisasm;
    hMenuDump = setupStruct->hMenuDump;
    hMenuStack = setupStruct->hMenuStack;
    
	GuiAddLogMessage, sztest_plugInfo;
	// place any additional setup code here
}


/*====================================================================================
  CBMENUENTRY - Called by debugger when a menu item is clicked - needs to be EXPORTED
 
  Arguments: cbType
             cbInfo - a pointer to a PLUG_CB_MENUENTRY structure. The hEntry contains 
             the resource id of menu item identifiers
   
  Notes:     hEntry can be used to determine if the user has clicked on your plugins
             menu item(s) and to do something in response to it.
             
--------------------------------------------------------------------------------------*/
extern "C" __declspec(dllexport) void CBMENUENTRY(CBTYPE cbType, PLUG_CB_MENUENTRY* info)
{
    switch(info->hEntry)
    {
    case MENU_TEST:
        if(!DbgIsDebugging())
        {
            _plugin_logputs("you need to be debugging to use this command");
            break;
        }
        break;
    }
}


/*====================================================================================
  CBINITDEBUG - Called by debugger when a program is debugged - needs to be EXPORTED

  Arguments: cbType
             cbInfo - a pointer to a PLUG_CB_INITDEBUG structure. 
             The szFileName item contains name of file being debugged. 

--------------------------------------------------------------------------------------*/
extern "C" __declspec(dllexport) void CBINITDEBUG(CBTYPE cbType, PLUG_CB_INITDEBUG* info)
{

}



/*====================================================================================
  CBSYSTEMBREAKPOINT - Called by debugger at system breakpoint - needs to be EXPORTED
 
  Arguments: cbType
             cbInfo - reserved 

--------------------------------------------------------------------------------------*/
extern "C" __declspec(dllexport) void CBSYSTEMBREAKPOINT(CBTYPE cbType, PLUG_CB_SYSTEMBREAKPOINT* info)
{

}














