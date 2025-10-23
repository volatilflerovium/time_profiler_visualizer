/*********************************************************************
* TimeProfilerVisualizer class                              			*
*                                                                    *
* Web based app to plot data output by                               *
* https://github.com/volatilflerovium/time_profiler_visualizer library *
*                                                                    *
* Version: 1.0                                                       *
* Date:    23-10-2025                                                *
* Author:  Dan Machado                                               *
**********************************************************************/
#include <filesystem>
#include <fstream>
#include <wx/wx.h>
#include <wx/webview.h>
#include <wx/icon.h>

//====================================================================

class TimeProfilerVisualizerApp : public wxFrame 
{
public:
	TimeProfilerVisualizerApp() 
	: wxFrame(nullptr, wxID_ANY, "Elapsed Time Visualizer", wxDefaultPosition, wxDefaultSize) 
	{
		std::string resourcePath="";
		#ifdef DEBUG
			resourcePath.append(DEBUG_DIR); // define in CMakeLists
		#else
		if(getenv("APPDIR")){
			resourcePath.append(getenv("APPDIR"));
			resourcePath.append("/usr/share");
		}
		else{
			throw std::runtime_error("Resource directory missing");
		}
		#endif

		std::string iconPath=resourcePath;
		#ifdef DEBUG
			iconPath.append("/icons/wxElapsedTimeVisualizer.png");
		#else
			iconPath.append("/icons/hicolor/256x256/apps/wxElapsedTimeVisualizer.png");
		#endif

		auto icon=wxIcon();
		icon.LoadFile(iconPath);
		SetIcon(icon);

		std::string htmlPath="file:";
		htmlPath.append(resourcePath);
		htmlPath.append("/elapsed_time_visualizer_files/lines.html");
		
		m_webViewPtr = wxWebView::New(this, wxID_ANY, htmlPath);

		m_webViewPtr->AddScriptMessageHandler("wx_msg");

		std::string cmdStr("xdg-open ");
		cmdStr.append(htmlPath);

		m_webViewPtr->Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, [cmdStr, this](wxWebViewEvent& evt) {
			if(evt.GetString()=="install"){
				autoInstall();
			}
			else{
				system(cmdStr.c_str());
			}
		});

		Maximize(true);
	}

	~TimeProfilerVisualizerApp()=default;

	private:
		wxWebView* m_webViewPtr;

		void autoInstall();
};

//--------------------------------------------------------------------

void TimeProfilerVisualizerApp::autoInstall()
{
#ifndef DEBUG
	const wxString desktopEntryFile=wxString::Format("%s/.local/share/applications/ElapsedTimeVisualizer.desktop", getenv("HOME")); 
	const wxString appInstallationDir=wxString::Format("%s/bin/appimages/ElapsedTimeVisualizer", getenv("HOME")); 
#else
	const wxString desktopEntryFile="/tmp/ElapsedTimeVisualizer.desktop";
	const wxString appInstallationDir="/tmp/bin/ElapsedTimeVisualizer";
#endif

	#ifndef DEBUG

	/*if(!install && !m_settings.canAutoInstallApp() ){
		return;
	}*/
	
	auto saveDataDialog=wxMessageDialog(
		this,
		wxT("Do you want to create a desktop file entry?"),
		wxT("Install"),
		wxYES_NO|wxCENTRE|wxICON_WARNING
	);

	int response=saveDataDialog.ShowModal();
	if(wxID_YES==response){
		std::string appImagePath=getenv("APPIMAGE");
		std::string appimageName=appImagePath.substr(appImagePath.find_last_of("/")+1);

		std::error_code ec;
		if(!std::filesystem::exists(std::string(appInstallationDir), ec)){
			std::filesystem::create_directories(std::string(appInstallationDir), ec);
		}
		
		const char* command="[Desktop Entry]\n\
Name=Time Profiler Visualizer\n\
Comment=Graphical tool for ploting data by \n\
Terminal=false\n\
Type=Application\n\
Exec=%s/%s\n\
Icon=%s/wxElapsedTimeVisualizer.png\n\
Categories=Development;";

		wxString fileContent=wxString::Format(command, appInstallationDir, appimageName, appInstallationDir);

		std::fstream fileStream(desktopEntryFile.mb_str(), std::ios::out | std::ios::trunc);
		if(fileStream.is_open()){
			fileStream<<fileContent;
			fileStream.close();
		}

		std::string srcFilePath=std::string(wxString::Format("%s/%s", getenv("OWD"), appimageName).mb_str());
		std::string dstFilePath=std::string(wxString::Format("%s/%s", appInstallationDir, appimageName).mb_str());

		std::filesystem::rename(srcFilePath, dstFilePath, ec);
		
		srcFilePath=std::string(wxString::Format("%s/wxElapsedTimeVisualizer.png", getenv("APPDIR")).mb_str());
		dstFilePath=std::string(wxString::Format("%s/wxElapsedTimeVisualizer.png", appInstallationDir).mb_str());

		if(std::filesystem::copy_file(srcFilePath, dstFilePath, std::filesystem::copy_options::overwrite_existing, ec)){
			m_webViewPtr->RunScript("document.appInstalled=true;");
			m_webViewPtr->RunScript("document.getElementById(\"installBtn\").disabled=true;");
		}
	}
	
	//m_settings.setInstallationStatus(wxID_YES==response);
	
	#endif
}

//====================================================================

class TimeProfilerVisualizer : public wxApp 
{
	bool OnInit() override 
	{
		return (new TimeProfilerVisualizerApp)->Show();
	}
};


wxIMPLEMENT_APP(TimeProfilerVisualizer);
