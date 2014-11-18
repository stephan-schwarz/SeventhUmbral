#include "AppConfig.h"
#include "AppPreferences.h"
#include "ZoneEditorWorldPane.h"
#include "resource.h"
#include "string_format.h"
#include "palleon\EmbedRemoteCall.h"

CZoneEditorWorldPane::CZoneEditorWorldPane(HWND parentWnd, uint32 mapId)
: Framework::Win32::CDialog(MAKEINTRESOURCE(IDD_ZONEEDITOR_WORLDPANE), parentWnd)
, m_mapId(mapId)
{
	SetClassPtr();
	CreateViewer();
}

CZoneEditorWorldPane::~CZoneEditorWorldPane()
{

}

void CZoneEditorWorldPane::CreateActor(uint32 actorId, uint32 baseModelId)
{
	Palleon::CEmbedRemoteCall rpc;
	rpc.SetMethod("CreateActor");
	rpc.SetParam("Id", std::to_string(actorId));
	rpc.SetParam("BaseModelId", std::to_string(baseModelId));
	m_embedControl->ExecuteCommand(rpc.ToString());
}

void CZoneEditorWorldPane::SetActive(bool active)
{
	m_embedControl->SetRunning(active);
}

long CZoneEditorWorldPane::OnSize(unsigned int, unsigned int, unsigned int)
{
	auto rect = GetClientRect();
	if(m_embedControl)
	{
		m_embedControl->SetSizePosition(rect);
	}
	return FALSE;
}

void CZoneEditorWorldPane::CreateViewer()
{
	assert(!m_embedControl);

	TCHAR moduleFileName[_MAX_PATH];
	GetModuleFileName(NULL, moduleFileName, _MAX_PATH);
	boost::filesystem::path dataPath(moduleFileName);
	dataPath.remove_leaf();
	dataPath /= "worldeditor";

	m_embedControl = std::make_shared<Palleon::CWin32EmbedControl>(m_hWnd, GetClientRect(),
		_T("WorldEditor.exe"), dataPath.native().c_str());

	m_embedControl->NotificationRaised.connect(NotificationRaised);

	{
		Palleon::CEmbedRemoteCall rpc;
		rpc.SetMethod("SetGamePath");
		rpc.SetParam("Path", CAppConfig::GetInstance().GetPreferenceString(PREF_WORKSHOP_GAME_LOCATION));
		m_embedControl->ExecuteCommand(rpc.ToString());
	}
	{
		Palleon::CEmbedRemoteCall rpc;
		rpc.SetMethod("SetMap");
		rpc.SetParam("MapId", std::to_string(m_mapId));
		auto result = m_embedControl->ExecuteCommand(rpc.ToString());
	}
}
