#include "be_cross.h"
#include "be_st.h"
#include "SDL_render.h"

void BEL_ST_ParseSetting_DisplayNum(const char *keyprefix, const char *buffer)
{
	if (sscanf(buffer, "%d", &g_refKeenCfg.displayNum) == 1)
		if ((g_refKeenCfg.displayNum < 0) || (g_refKeenCfg.displayNum >= SDL_GetNumVideoDisplays()))
			g_refKeenCfg.displayNum = 0;
}

void BEL_ST_ParseSetting_SDLRendererDriver(const char *keyprefix, const char *buffer)
{
	SDL_RendererInfo info;
	for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i)
	{
		SDL_GetRenderDriverInfo(i, &info);
		if (!strcmp(info.name, buffer))
		{
			g_refKeenCfg.sdlRendererDriver = i;
			break;
		}
	}
}

void BEL_ST_SaveSDLRendererDriverToConfig(FILE *fp, const char *keyprefix, int driver)
{
	if (driver < 0)
		fprintf(fp, "%sauto\n", keyprefix);
	else
	{
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(g_refKeenCfg.sdlRendererDriver, &info); // g_refKeenCfg.sdlRendererDriver should be a valid renderer driver index here
		fprintf(fp, "%s%s\n", keyprefix, info.name);
	}
}
