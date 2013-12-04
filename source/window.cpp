#ifdef WIN32

#include "headers.h"
#include "memory.h"
#include "resource.h"
#include "serverlist.h"

HINSTANCE gHinst;
HWND mainWnd;
NOTIFYICONDATA nidTray;
ServerList *serverList;
extern Memory *memory;

void DoTrayIcon(HWND hWnd)
{
	nidTray.cbSize = sizeof(NOTIFYICONDATA);
	nidTray.hIcon = LoadIcon(gHinst, MAKEINTRESOURCE(ID_ICON32));
	nidTray.hWnd = hWnd;
	nidTray.uCallbackMessage = WM_SHELLNOTIFY;
	lstrcpy(nidTray.szTip, "OTNet Ipchanger");
	nidTray.uID = ID_TRAY;
	nidTray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	ShowWindow(hWnd, SW_HIDE);
	Shell_NotifyIcon(NIM_ADD,&nidTray);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static char ip[256];
    static uint16_t port;
    
    switch(uMsg)
    {
        case WM_INITDIALOG: 
        {
			SendDlgItemMessage(hWnd, ID_IPTEXT, CB_RESETCONTENT, 0, 0);
			
			std::list<Server*> mServerList = serverList->getServerList();
			for(std::list<Server*>::iterator it = mServerList.begin(), end = mServerList.end(); it != end; it++)
				SendDlgItemMessage(hWnd, ID_IPTEXT, CB_ADDSTRING, 0, (LPARAM)(*it)->text.c_str());
				
            SetDlgItemText(hWnd,ID_IPTEXT,mServerList.front()->ip);
            SetDlgItemInt(hWnd,ID_PORTTEXT,mServerList.front()->port,true);
            
            break;
        }
        case WM_SHELLNOTIFY:
		{
			if(lParam == WM_RBUTTONDOWN || lParam == WM_LBUTTONDOWN)
			{
                Shell_NotifyIcon(NIM_DELETE,&nidTray);
                ShowWindow(mainWnd, SW_SHOW);
            }
            break;
        }
        case WM_DESTROY:
        case WM_CLOSE:
		case WM_QUERYENDSESSION:
		{
            Shell_NotifyIcon(NIM_DELETE,&nidTray);
            PostQuitMessage(0);
			break;
        }
        case WM_MEASUREITEM:
		{
			if(wParam == ID_IPTEXT)
			{
				MEASUREITEMSTRUCT *lpMeasureItem = (LPMEASUREITEMSTRUCT)lParam;
				lpMeasureItem->itemHeight = 15;
			}
			break;
		}
        case WM_DRAWITEM:
		{
			if(wParam == ID_IPTEXT)
			{
				DRAWITEMSTRUCT *lpDrawItem = (LPDRAWITEMSTRUCT)lParam;

				if(lpDrawItem->itemAction & ODA_DRAWENTIRE)
				{
					DrawText(lpDrawItem->hDC, (char*)lpDrawItem->itemData, strlen((char*)lpDrawItem->itemData), &lpDrawItem->rcItem, DT_LEFT);
					if(lpDrawItem->itemState & ODS_SELECTED)
						DrawFocusRect(lpDrawItem->hDC, &lpDrawItem->rcItem);
				}
				if(lpDrawItem->itemAction & ODA_SELECT)
					DrawFocusRect(lpDrawItem->hDC, &lpDrawItem->rcItem);
			}
			break;
		}
        case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case ID_IPTEXT:
				{
					switch(HIWORD(wParam))
					{
						case CBN_SELCHANGE:
						{
							uint32_t nItem = SendDlgItemMessage(hWnd, ID_IPTEXT, CB_GETCURSEL, 0, 0), x = 0;
							
							std::list<Server*> mServerList = serverList->getServerList();
							for(std::list<Server*>::iterator it = mServerList.begin(), end = mServerList.end(); it != end; it++)
							{
								if(x == nItem)
								{
									SetDlgItemText(hWnd,ID_IPTEXT,(*it)->ip);
									SetDlgItemInt(hWnd,ID_PORTTEXT,(*it)->port,true);
									break;
								}
								x++;
							}
							break;
						}
					}
					break;
				}
			}
			switch(wParam)
			{
                case ID_CHANGE:
                {
                    GetDlgItemText(hWnd, ID_IPTEXT,ip,256);
                    port = GetDlgItemInt(hWnd, ID_PORTTEXT,false,true);
                    
                    if(!memory->injectTibia())
                        break;

                    if(memory->changeIp(ip, port))
                    {     
						Server *server = new Server();
						sprintf(server->ip,"%s",ip);
						server->port = port;
						
						uint8_t manager = serverList->add(server);
						if(manager == 1)
						{
							SendDlgItemMessage(hWnd, ID_IPTEXT, CB_INSERTSTRING, 0, (LPARAM)server->text.c_str());
						}
						else if(manager == 2)
						{
							SendDlgItemMessage(hWnd, ID_IPTEXT, CB_INSERTSTRING, 0, (LPARAM)server->text.c_str());
							SendDlgItemMessage(hWnd, ID_IPTEXT, CB_DELETESTRING, serverList->getStoredIps(), 0);
						}
                    }
                    
                    memory->desinjectTibia();
                    break;
                }
                case ID_HIDE:
                {
                    DoTrayIcon(mainWnd);
                    break;
                }
            }
            break;
        }
    }
    return 0;
}

bool initializeWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
	serverList = new ServerList();
	if(!serverList->load())
		return true;
				
	WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize			= sizeof(WNDCLASSEX);
    wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc		= WndProc;
    wc.hInstance		= hInstance;
    wc.hbrBackground	=(HBRUSH)(COLOR_APPWORKSPACE);
    wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName	= "otnetipc";
    wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(ID_ICON32));
    wc.hIconSm			= LoadIcon(hInstance, MAKEINTRESOURCE(ID_ICON16));
	
	if(!RegisterClassEx(&wc))
	{
        MessageBox(NULL, "Window registration failed.", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }
    
    gHinst = hInstance;
    mainWnd = CreateDialog(hInstance, MAKEINTRESOURCE(ID_MAIN), NULL, (DLGPROC)WndProc);
    SendMessage(mainWnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(gHinst, MAKEINTRESOURCE(ID_ICON16)));
    
    MSG msg;
    while(GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	UnregisterClass("otnetmipc",hInstance);
	return msg.wParam;
}

#endif
