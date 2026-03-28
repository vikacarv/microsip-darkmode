/*
 * Copyright (C) 2011-2024 MicroSIP (http://www.microsip.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "StdAfx.h"
#include "RinginDlg.h"
#include "langpack.h"
#include "mainDlg.h"
#include "settings.h"
#include "Markup.h"
// ── FASE 14: Dark Mode ───────────────────────────────
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
// ─────────────────────────────────────────────────────
#include <vector>
#include <algorithm>

RinginDlg::RinginDlg(CWnd *pParent /*=NULL*/)
	: CBaseDialog(RinginDlg::IDD, pParent)
{
	Create(IDD, pParent);
	answered = false;
}

RinginDlg::~RinginDlg(void)
{
}

int RinginDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (langPack.rtl)
	{
		ModifyStyleEx(0, WS_EX_LAYOUTRTL);
	}
	return 0;
}

BOOL CALLBACK MyInfoEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFOEX iMonitor;
	iMonitor.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &iMonitor);
	if (iMonitor.dwFlags == DISPLAY_DEVICE_MIRRORING_DRIVER)
	{
		return true;
	}
	else
	{
		reinterpret_cast<std::vector<HMONITOR> *>(dwData)->push_back(hMonitor);
		return true;
	}
}

void RinginDlg::DoDataExchange(CDataExchange *pDX)
{
	CBaseDialog::DoDataExchange(pDX);
}

BOOL RinginDlg::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	// ── Dark Mode ────────────────────────────────────────────────────────────
	// Barra de título escura
	BOOL darkTitle = TRUE;
	DwmSetWindowAttribute(GetSafeHwnd(), 20, &darkTitle, sizeof(darkTitle));

	// Aplicar BS_OWNERDRAW em todos os push buttons
	{
		CWnd *pChild = GetWindow(GW_CHILD);
		while (pChild)
		{
			TCHAR szClass[64] = {0};
			::GetClassName(pChild->GetSafeHwnd(), szClass, 63);
			if (_tcsicmp(szClass, _T("Button")) == 0)
			{
				LONG style = ::GetWindowLong(pChild->GetSafeHwnd(), GWL_STYLE);
				LONG tipo = style & BS_TYPEMASK;
				if (tipo == BS_PUSHBUTTON || tipo == BS_DEFPUSHBUTTON)
				{
					LONG lStyle = style;
					lStyle &= ~(BS_PUSHBUTTON | BS_DEFPUSHBUTTON);
					lStyle |= BS_OWNERDRAW;
					::SetWindowLong(pChild->GetSafeHwnd(), GWL_STYLE, lStyle);
					SetWindowTheme(pChild->GetSafeHwnd(), L"", L"");
				}
			}
			pChild = pChild->GetNextWindow();
		}
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	// ── Fim Dark Mode ─────────────────────────────────────────────────────────
	AutoMove(IDC_ANSWER, 0, 100, 0, 0);
	AutoMove(IDC_DECLINE, 0, 100, 0, 0);
	AutoMove(IDC_IGNORE, 0, 100, 0, 0);

#ifdef _GLOBAL_VIDEO
	if (accountSettings.disableVideo)
	{
		GetDlgItem(IDC_VIDEO)->ShowWindow(SW_HIDE);
	}
#endif

	TranslateDialog(this->m_hWnd);

	CFont *font = this->GetFont();
	LOGFONT lf;
	font->GetLogFont(&lf);

	lf.lfHeight = -MulDiv(10, dpiY, 96);
	m_font_ignore.CreateFontIndirect(&lf);
	GetDlgItem(IDC_IGNORE)->SetFont(&m_font_ignore);
	GetDlgItem(IDC_IGNORE)->EnableWindow(FALSE);
	if (accountSettings.noIgnoreCall)
	{
		GetDlgItem(IDC_IGNORE)->ShowWindow(SW_HIDE);
	}
	lf.lfHeight = -MulDiv(20, dpiY, 96);
	lf.lfWeight = FW_BOLD;
	m_font.CreateFontIndirect(&lf);
	GetDlgItem(IDC_CALLER_NAME)->SetFont(&m_font);

	GetDlgItem(IDC_CALLER_NAME)->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_NOSIZE | SWP_FRAMECHANGED);
	GetDlgItem(IDC_CALLER_ADDR)->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_NOSIZE | SWP_FRAMECHANGED);

	m_hIconTransfer = LoadImageIcon(IDI_FORWARD, 16, 16);
	((CButton *)GetDlgItem(IDC_TRANSFER))->SetIcon(m_hIconTransfer);
	int x, y;
	if (accountSettings.randomAnswerBox)
	{
		CRect ringinRect;
		GetWindowRect(&ringinRect);
		if (accountSettings.multiMonitor)
		{
			std::vector<HMONITOR> hMonitorArray;
			EnumDisplayMonitors(NULL, NULL, &MyInfoEnumProc, reinterpret_cast<LPARAM>(&hMonitorArray));
			std::random_shuffle(hMonitorArray.begin(), hMonitorArray.end());
			std::vector<HMONITOR>::iterator it = hMonitorArray.begin();
			HMONITOR hMonitor = *it;
			MONITORINFO mi;
			mi.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(hMonitor, &mi);
			x = mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left) - ringinRect.Width()) * rand() / RAND_MAX;
			y = mi.rcWork.top + ((mi.rcWork.bottom - mi.rcWork.top) - ringinRect.Height()) * rand() / RAND_MAX;
		}
		else
		{
			CRect primaryScreenRect;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &primaryScreenRect, 0);
			x = primaryScreenRect.left + ((primaryScreenRect.right - primaryScreenRect.left) - ringinRect.Width()) * rand() / RAND_MAX;
			y = primaryScreenRect.top + ((primaryScreenRect.bottom - primaryScreenRect.top) - ringinRect.Height()) * rand() / RAND_MAX;
		}
	}
	else
	{
		if (mainDlg->ringinDlgs.GetCount())
		{
			CRect rect;
			mainDlg->ringinDlgs.GetAt(mainDlg->ringinDlgs.GetCount() - 1)->GetWindowRect(&rect);
			x = rect.left + 22;
			y = rect.top + 22;
		}
		else
		{
			if (accountSettings.ringinX || accountSettings.ringinY)
			{
				CRect screenRect;
				MSIP::GetScreenRect(&screenRect);
				if (accountSettings.multiMonitor)
				{
					MSIP::GetScreenRect(&screenRect);
				}
				else
				{
					SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);
				}
				CRect rect;
				GetWindowRect(&rect);
				int maxLeft = screenRect.right - rect.Width();
				if (accountSettings.ringinX > maxLeft)
				{
					x = maxLeft;
				}
				else
				{
					x = accountSettings.ringinX < screenRect.left ? screenRect.left : accountSettings.ringinX;
				}
				int maxTop = screenRect.bottom - rect.Height();
				if (accountSettings.ringinY > maxTop)
				{
					y = maxTop;
				}
				else
				{
					y = accountSettings.ringinY < screenRect.top ? screenRect.top : accountSettings.ringinY;
				}
			}
			else
			{
				CRect ringinRect;
				GetWindowRect(&ringinRect);
				CRect primaryScreenRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &primaryScreenRect, 0);
				x = (primaryScreenRect.Width() - ringinRect.Width()) / 2;
				y = (primaryScreenRect.Height() - ringinRect.Height()) / 2;
			}
		}
	}
	SetWindowPos(accountSettings.bringToFrontOnIncoming || accountSettings.alwaysOnTop ? &this->wndTopMost : &this->wndNoTopMost, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
	if (accountSettings.bringToFrontOnIncoming)
	{

		if (mainDlg->IsWindowVisible())
		{
			if (mainDlg->IsIconic())
			{
				mainDlg->ShowWindow(SW_RESTORE);
			}
			else
			{
				mainDlg->ShowWindow(SW_HIDE);
				mainDlg->ShowWindow(SW_MINIMIZE);
				mainDlg->ShowWindow(SW_RESTORE);
			}
		}
		ShowWindow(SW_SHOWNORMAL);
		SetForegroundWindow();
	}
	else
	{
		if (mainDlg->IsWindowVisible())
		{
			ShowWindow(SW_SHOWNORMAL);
		}
	}
	return 0;
}

void RinginDlg::SetCallId(pjsua_call_id new_call_id)
{
	call_id = new_call_id;
}

BEGIN_MESSAGE_MAP(RinginDlg, CBaseDialog)
ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_TIMER()
ON_WM_MOVE()
ON_WM_SHOWWINDOW()
ON_WM_DRAWITEM()
ON_BN_CLICKED(IDOK, &RinginDlg::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &RinginDlg::OnBnClickedCancel)
ON_BN_CLICKED(IDC_ANSWER, &RinginDlg::OnBnClickedAudio)
ON_BN_CLICKED(IDC_DECLINE, &RinginDlg::OnBnClickedDecline)
ON_BN_CLICKED(IDC_VIDEO, &RinginDlg::OnBnClickedVideo)
ON_BN_CLICKED(IDC_TRANSFER, OnBnClickedTransfer)
END_MESSAGE_MAP()

void RinginDlg::OnClose()
{
	if (accountSettings.noIgnoreCall)
	{
		OnBnClickedDecline();
	}
	else
	{
		Close();
	}
}

void RinginDlg::OnAnswer()
{
	answered = true;
#ifdef _GLOBAL_VIDEO
	GetDlgItem(IDC_VIDEO)->EnableWindow(FALSE);
#endif
	GetDlgItem(IDC_ANSWER)->EnableWindow(FALSE);
	GetDlgItem(IDC_DECLINE)->EnableWindow(FALSE);
}

void RinginDlg::Close(BOOL accept)
{
	int count = mainDlg->ringinDlgs.GetCount();
	for (int i = 0; i < count; i++)
	{
		if (call_id == mainDlg->ringinDlgs.GetAt(i)->call_id)
		{
			if (!accept)
			{
				mainDlg->UpdateWindowText(_T("-"));
			}
			if (count == 1)
			{
				mainDlg->PlayerStop();
			}
			mainDlg->ringinDlgs.RemoveAt(i);
			call_id = -1;
			break;
		}
	}
	if (call_id == -1)
	{
		DestroyWindow();
	}
}

void RinginDlg::OnBnClickedOk()
{
}

void RinginDlg::OnBnClickedCancel()
{
	Close();
}

void RinginDlg::OnBnClickedDecline()
{
	if (!answered)
	{
		pjsua_call_info call_info;
		pjsua_call_get_info(call_id, &call_info);
		call_user_data *user_data = (call_user_data *)pjsua_call_get_user_data(call_id);
		if (user_data)
		{
			user_data->CS.Lock();
			user_data->hangup = true;
			user_data->CS.Unlock();
		}
		msip_call_busy(call_id);
		mainDlg->callIdIncomingIgnore = MSIP::PjToStr(&call_info.call_id);
	}
	Close();
}

void RinginDlg::OnBnClickedAudio()
{
	CallAccept();
}

void RinginDlg::OnBnClickedVideo()
{
	CallAccept(TRUE);
}

void RinginDlg::CallAccept(BOOL hasVideo)
{
	if (!answered)
	{
		mainDlg->onCallAnswer((WPARAM)call_id, (LPARAM)hasVideo);
	}
}

void RinginDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	SetTimer(IDT_TIMER_INIT_RINGIN, 1000, NULL);
}

void RinginDlg::OnTimer(UINT_PTR TimerVal)
{
	if (TimerVal == IDT_TIMER_INIT_RINGIN)
	{
		KillTimer(IDT_TIMER_INIT_RINGIN);
	}
}

void RinginDlg::OnMove(int x, int y)
{
	if (IsWindowVisible() && !IsZoomed() && !IsIconic())
	{
		CRect cRect;
		GetWindowRect(&cRect);
		accountSettings.ringinX = cRect.left;
		accountSettings.ringinY = cRect.top;
		mainDlg->AccountSettingsPendingSave();
	}
}

void RinginDlg::OnBnClickedTransfer()
{
	mainDlg->OpenTransferDlg(this, MSIP_ACTION_FORWARD, call_id);
}
// ── FASE 14: Dark Mode ───────────────────────────────────────────────────────

BOOL RinginDlg::OnEraseBkgnd(CDC *pDC)
{
	CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(&rect, RGB(28, 28, 28));
	return TRUE;
}

void RinginDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS)
{
	if (lpDIS->CtlType != ODT_BUTTON)
	{
		CBaseDialog::OnDrawItem(nIDCtl, lpDIS);
		return;
	}

	HDC hdc = lpDIS->hDC;
	RECT rc = lpDIS->rcItem;
	UINT state = lpDIS->itemState;

	bool pressed = (state & ODS_SELECTED) != 0;
	bool focused = (state & ODS_FOCUS) != 0;
	bool disabled = (state & ODS_DISABLED) != 0;

	// Botão "Atender" (IDC_ANSWER) recebe cor verde cyberpunk quando pressionado
	// Botão "Recusar" (IDC_DECLINE) recebe cor rosa cyberpunk quando pressionado
	// Demais botões seguem o padrão da Fase 14
	COLORREF clrBg, clrBord, clrText;

	if (nIDCtl == IDC_ANSWER)
	{
		clrBg = pressed ? RGB(0, 120, 60) : disabled ? RGB(28, 28, 28)
													 : RGB(36, 36, 36);
		clrBord = focused ? RGB(180, 20, 90) : pressed ? RGB(0, 200, 80)
													   : RGB(80, 80, 80);
		clrText = pressed ? RGB(180, 255, 180) : disabled ? RGB(80, 80, 80)
														  : RGB(210, 210, 210);
	}
	else if (nIDCtl == IDC_DECLINE)
	{
		clrBg = pressed ? RGB(120, 0, 60) : disabled ? RGB(28, 28, 28)
													 : RGB(36, 36, 36);
		clrBord = focused ? RGB(180, 20, 90) : pressed ? RGB(180, 20, 90)
													   : RGB(80, 80, 80);
		clrText = pressed ? RGB(255, 20, 147) : disabled ? RGB(80, 80, 80)
														 : RGB(210, 210, 210);
	}
	else
	{
		clrBg = pressed ? RGB(120, 0, 60) : disabled ? RGB(28, 28, 28)
													 : RGB(36, 36, 36);
		clrBord = focused ? RGB(180, 20, 90) : pressed ? RGB(180, 20, 90)
													   : RGB(80, 80, 80);
		clrText = pressed ? RGB(255, 20, 147) : disabled ? RGB(80, 80, 80)
														 : RGB(210, 210, 210);
	}

	// Fundo
	HBRUSH hBrush = ::CreateSolidBrush(clrBg);
	::FillRect(hdc, &rc, hBrush);
	::DeleteObject(hBrush);

	// Borda
	HPEN hPen = ::CreatePen(PS_SOLID, 1, clrBord);
	HPEN hOldPen = (HPEN)::SelectObject(hdc, hPen);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, ::GetStockObject(HOLLOW_BRUSH));
	::Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
	::SelectObject(hdc, hOldPen);
	::SelectObject(hdc, hOldBrush);
	::DeleteObject(hPen);

	// Ícone (se houver — ex: IDC_TRANSFER com ícone de telefone)
	HICON hIcon = (HICON)::SendMessage(lpDIS->hwndItem, BM_GETIMAGE, IMAGE_ICON, 0);
	if (hIcon)
	{
		ICONINFO ii = {0};
		::GetIconInfo(hIcon, &ii);
		BITMAP bm = {0};
		::GetObject(ii.hbmColor ? ii.hbmColor : ii.hbmMask, sizeof(bm), &bm);
		int iconW = bm.bmWidth;
		int iconH = bm.bmHeight;
		if (ii.hbmColor)
			::DeleteObject(ii.hbmColor);
		if (ii.hbmMask)
			::DeleteObject(ii.hbmMask);

		int x = rc.left + (rc.right - rc.left - iconW) / 2;
		int y = rc.top + (rc.bottom - rc.top - iconH) / 2;
		::DrawIconEx(hdc, x, y, hIcon, iconW, iconH, 0, NULL, DI_NORMAL);
		return; // não desenha texto se tem ícone
	}

	// Texto
	TCHAR szText[128] = {0};
	::GetWindowText(lpDIS->hwndItem, szText, 127);
	if (szText[0])
	{
		::SetBkMode(hdc, TRANSPARENT);
		::SetTextColor(hdc, clrText);
		HFONT hFont = (HFONT)::SendMessage(lpDIS->hwndItem, WM_GETFONT, 0, 0);
		HFONT hOldFont = (HFONT)::SelectObject(hdc, hFont);
		::DrawText(hdc, szText, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		::SelectObject(hdc, hOldFont);
	}
}

// ─────────────────────────────────────────────────────────────────────────────