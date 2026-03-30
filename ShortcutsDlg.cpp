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
#include "ShortcutsDlg.h"
#include "mainDlg.h"
#include "settings.h"
#include "langpack.h"
// ── FASE 16: Dark Mode ───────────────────────────────
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")
// ─────────────────────────────────────────────────────

static int numberWidth = -1;
static int numberHeight = -1;

static CString defaultActionItems[] = {
	MSIP_SHORTCUT_CALL,
#ifdef _GLOBAL_VIDEO
	MSIP_SHORTCUT_VIDEOCALL,
#endif
	MSIP_SHORTCUT_MESSAGE,
	MSIP_SHORTCUT_DTMF,
	MSIP_SHORTCUT_TRANSFER,
	MSIP_SHORTCUT_ATTENDED_TRANSFER,
	MSIP_SHORTCUT_CONFERENCE,
	MSIP_SHORTCUT_RUNBATCH,
	MSIP_SHORTCUT_CALL_URL,
	MSIP_SHORTCUT_POP_URL,
};
static CString defaultActionValues[] = {
	_T("Call"),
#ifdef _GLOBAL_VIDEO
	_T("Video Call"),
#endif
	_T("Message"),
	_T("DTMF"),
	_T("Call Transfer"),
	_T("Attended Transfer"),
	_T("Conference"),
	_T("Run Batch"),
	_T("Call URL"),
	_T("Pop URL"),
};

ShortcutsDlg::ShortcutsDlg(CWnd *pParent /*=NULL*/)
	: CDialog(ShortcutsDlg::IDD, pParent)
{
	if (!Create(IDD, pParent))
	{
		AfxMessageBox(_T("Failed to create shortcuts window on your system"));
		exit(0);
	}
}

ShortcutsDlg::~ShortcutsDlg(void)
{
}

int ShortcutsDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (langPack.rtl)
	{
		ModifyStyleEx(0, WS_EX_LAYOUTRTL);
	}
	return 0;
}

BOOL ShortcutsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	TranslateDialog(this->m_hWnd);

	CString str;
	GetDlgItem(IDC_SHORTCUTS_NUMBER2)->GetWindowText(str);
	str.Format(_T("%s 2"), Translate(str.GetBuffer()));
	GetDlgItem(IDC_SHORTCUTS_NUMBER2)->SetWindowText(str);

	CRect rect;
	GetDlgItem(IDC_SHORTCUTS_EDIT_SHORTCUT1_NUMBER)->GetWindowRect(rect);
	numberWidth = rect.Width();
	numberHeight = rect.Height();

	((CButton *)GetDlgItem(IDC_SHORTCUTS_ENABLE))->SetCheck(accountSettings.enableShortcuts);
	((CButton *)GetDlgItem(IDC_SHORTCUTS_BOTTOM))->SetCheck(accountSettings.shortcutsBottom);

	CComboBox *combobox;
	for (int i = 0; i < _GLOBAL_SHORTCUTS_QTY; i++)
	{
		combobox = (CComboBox *)GetDlgItem(IDC_SHORTCUTS_COMBO_SHORTCUT1_TYPE + i * 6);
		Shortcut shortcut;
		if (i < shortcuts.GetCount())
		{
			shortcut = shortcuts.GetAt(i);
			GetDlgItem(IDC_SHORTCUTS_EDIT_SHORTCUT1_LABEL + i * 6)->SetWindowText(shortcut.label);
			GetDlgItem(IDC_SHORTCUTS_EDIT_SHORTCUT1_NUMBER + i * 6)->SetWindowText(shortcut.number);
			GetDlgItem(IDC_SHORTCUTS_EDIT_SHORTCUT1_NUMBER2 + i * 6)->SetWindowText(shortcut.number2);
		}
		int n = sizeof(defaultActionItems) / sizeof(defaultActionItems[0]);
		int c = 0;
		for (int j = 0; j < n; j++)
		{
			CString str = defaultActionValues[j];
			if (defaultActionItems[j] == _T("runBatch") || defaultActionItems[j] == _T("callURL") || defaultActionItems[j] == _T("popURL"))
			{
				str.Append(_T("*"));
			}
			combobox->AddString(Translate(str.GetBuffer()));
			if (shortcut.type == defaultActionItems[j])
			{
				c = j;
			}
		}
		combobox->SetCurSel(c);
		if (!shortcut.number2.IsEmpty())
		{
			CButton *checkbox = (CButton *)GetDlgItem(IDC_SHORTCUTS_TOGGLE1 + i * 6);
			checkbox->SetCheck(1);
			UpdateToggle(true, i);
		}
		CButton *checkbox = (CButton *)GetDlgItem(IDC_SHORTCUTS_PRESENCE1 + i * 6);
		checkbox->SetCheck(shortcut.presence);
	}

	// ── FASE 16: Dark Mode ───────────────────────────────────────────────────

	// Barra de título escura
	BOOL darkTitle = TRUE;
	DwmSetWindowAttribute(GetSafeHwnd(), 20, &darkTitle, sizeof(darkTitle));

	// Fundo da janela
	SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND,
					(LONG_PTR)::CreateSolidBrush(RGB(28, 28, 28)));

	// Loop em todos os controles filhos
	{
		CWnd *pChild = GetWindow(GW_CHILD);
		while (pChild)
		{
			TCHAR szClass[64] = {0};
			::GetClassName(pChild->GetSafeHwnd(), szClass, 63);

			if (_tcsicmp(szClass, _T("ComboBox")) == 0)
			{
				// CBS_OWNERDRAWFIXED — OnDrawItem assume 100% do desenho
				LONG lStyle = ::GetWindowLong(pChild->GetSafeHwnd(), GWL_STYLE);
				lStyle &= ~(CBS_OWNERDRAWVARIABLE);
				lStyle |= CBS_OWNERDRAWFIXED | CBS_HASSTRINGS;
				::SetWindowLong(pChild->GetSafeHwnd(), GWL_STYLE, lStyle);
				SetWindowTheme(pChild->GetSafeHwnd(), L"", L"");
			}
			else if (_tcsicmp(szClass, _T("Button")) == 0)
			{
				LONG style = ::GetWindowLong(pChild->GetSafeHwnd(), GWL_STYLE);
				LONG tipo = style & BS_TYPEMASK;
				if (tipo == BS_PUSHBUTTON || tipo == BS_DEFPUSHBUTTON)
				{
					// BS_OWNERDRAW — OnDrawItem assume 100% do desenho
					LONG lStyle = ::GetWindowLong(pChild->GetSafeHwnd(), GWL_STYLE);
					lStyle &= ~(BS_PUSHBUTTON | BS_DEFPUSHBUTTON);
					lStyle |= BS_OWNERDRAW;
					::SetWindowLong(pChild->GetSafeHwnd(), GWL_STYLE, lStyle);
					SetWindowTheme(pChild->GetSafeHwnd(), L"", L"");
				}
				else
				{
					// Checkboxes — remove tema para OnCtlColor funcionar
					SetWindowTheme(pChild->GetSafeHwnd(), L"", L"");
				}
			}
			else if (_tcsicmp(szClass, _T("Edit")) == 0)
			{
				SetWindowTheme(pChild->GetSafeHwnd(), L"", L"");
			}

			pChild = pChild->GetNextWindow();
		}
	}

	// OBRIGATÓRIO: forçar repintura após owner-draw aplicado em runtime
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

	// ── Fim Fase 16 ──────────────────────────────────────────────────────────

	return TRUE;
}

void ShortcutsDlg::OnDestroy()
{
	mainDlg->shortcutsDlg = NULL;
	CDialog::OnDestroy();
}

void ShortcutsDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}

BEGIN_MESSAGE_MAP(ShortcutsDlg, CDialog)
ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_SHORTCUTS_TOGGLE1, OnBnClickedToggle)
ON_BN_CLICKED(IDC_SHORTCUTS_TOGGLE2, OnBnClickedToggle)
ON_BN_CLICKED(IDC_SHORTCUTS_TOGGLE3, OnBnClickedToggle)
ON_BN_CLICKED(IDC_SHORTCUTS_TOGGLE4, OnBnClickedToggle)
ON_BN_CLICKED(IDC_SHORTCUTS_TOGGLE5, OnBnClickedToggle)
ON_BN_CLICKED(IDC_SHORTCUTS_TOGGLE6, OnBnClickedToggle)
ON_BN_CLICKED(IDC_SHORTCUTS_TOGGLE7, OnBnClickedToggle)
ON_BN_CLICKED(IDC_SHORTCUTS_TOGGLE8, OnBnClickedToggle)
ON_NOTIFY(NM_CLICK, IDC_SHORTCUTS_SYSLINK_TOGGLE, &ShortcutsDlg::OnNMClickSyslinkToggle)
ON_NOTIFY(NM_CLICK, IDC_SHORTCUTS_SYSLINK_BLF, &ShortcutsDlg::OnNMClickSyslinkBLF)
ON_BN_CLICKED(IDCANCEL, &ShortcutsDlg::OnBnClickedCancel)
ON_BN_CLICKED(IDOK, &ShortcutsDlg::OnBnClickedOk)
// ── FASE 16: Dark Mode ───────────────────────────
ON_WM_CTLCOLOR()
ON_WM_ERASEBKGND()
ON_WM_DRAWITEM()
// ─────────────────────────────────────────────────
END_MESSAGE_MAP()

void ShortcutsDlg::OnClose()
{
	DestroyWindow();
}

void ShortcutsDlg::OnBnClickedCancel()
{
	OnClose();
}

void ShortcutsDlg::OnBnClickedOk()
{
	this->ShowWindow(SW_HIDE);
	mainDlg->ShortcutsRemoveAll();
	for (int i = 0; i < _GLOBAL_SHORTCUTS_QTY; i++)
	{
		Shortcut shortcut;
		GetDlgItem(IDC_SHORTCUTS_EDIT_SHORTCUT1_LABEL + i * 6)->GetWindowText(shortcut.label);
		GetDlgItem(IDC_SHORTCUTS_EDIT_SHORTCUT1_NUMBER + i * 6)->GetWindowText(shortcut.number);
		GetDlgItem(IDC_SHORTCUTS_EDIT_SHORTCUT1_NUMBER2 + i * 6)->GetWindowText(shortcut.number2);
		CButton *checkbox = (CButton *)GetDlgItem(IDC_SHORTCUTS_TOGGLE1 + i * 6);
		if (!checkbox->GetCheck())
		{
			shortcut.number2.Empty();
		}
		shortcut.presence = ((CButton *)GetDlgItem(IDC_SHORTCUTS_PRESENCE1 + i * 6))->GetState();
		int n = ((CComboBox *)GetDlgItem(IDC_SHORTCUTS_COMBO_SHORTCUT1_TYPE + i * 6))->GetCurSel();
		if (n >= 0 && !shortcut.label.IsEmpty() &&
			(!shortcut.number.IsEmpty() ||
			 defaultActionItems[n] == MSIP_SHORTCUT_TRANSFER ||
			 defaultActionItems[n] == MSIP_SHORTCUT_ATTENDED_TRANSFER))
		{
			shortcut.type = defaultActionItems[n];
			shortcuts.Add(shortcut);
		}
	}
	ShortcutsSave();

	accountSettings.enableShortcuts = ((CButton *)GetDlgItem(IDC_SHORTCUTS_ENABLE))->GetCheck();
	accountSettings.shortcutsBottom = ((CButton *)GetDlgItem(IDC_SHORTCUTS_BOTTOM))->GetCheck();

	mainDlg->pageDialer->RebuildShortcutsRestart();

	OnClose();
}

void ShortcutsDlg::OnBnClickedToggle()
{
	CComboBox *combobox;
	bool hasChecked = false;
	for (int i = 0; i < _GLOBAL_SHORTCUTS_QTY; i++)
	{
		CButton *checkbox = (CButton *)GetDlgItem(IDC_SHORTCUTS_TOGGLE1 + i * 6);
		if (checkbox == GetFocus())
		{
			UpdateToggle(checkbox->GetCheck(), i);
		}
		if (!hasChecked && checkbox->GetCheck())
		{
			hasChecked = true;
		}
	}
	if (!hasChecked)
	{
		GetDlgItem(IDC_SHORTCUTS_NUMBER2)->ShowWindow(SW_HIDE);
	}
}

void ShortcutsDlg::OnNMClickSyslinkToggle(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("toggle"));
	*pResult = 0;
}

void ShortcutsDlg::OnNMClickSyslinkBLF(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("BLF"));
	*pResult = 0;
}

void ShortcutsDlg::UpdateToggle(bool check, int i)
{
	if (check)
	{
		GetDlgItem(IDC_SHORTCUTS_EDIT_SHORTCUT1_NUMBER + i * 6)->SetWindowPos(NULL, 0, 0, numberWidth / 2, numberHeight, SWP_NOMOVE | SWP_NOZORDER);
		GetDlgItem(IDC_SHORTCUTS_EDIT_SHORTCUT1_NUMBER2 + i * 6)->ShowWindow(SW_NORMAL);
		GetDlgItem(IDC_SHORTCUTS_NUMBER2)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_SHORTCUTS_EDIT_SHORTCUT1_NUMBER + i * 6)->SetWindowPos(NULL, 0, 0, numberWidth, numberHeight, SWP_NOMOVE | SWP_NOZORDER);
		GetDlgItem(IDC_SHORTCUTS_EDIT_SHORTCUT1_NUMBER2 + i * 6)->ShowWindow(SW_HIDE);
	}
}

// ── FASE 16: Dark Mode ───────────────────────────────────────────────────────

HBRUSH ShortcutsDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	static HBRUSH hBrushDark = ::CreateSolidBrush(RGB(28, 28, 28));
	static HBRUSH hBrushControl = ::CreateSolidBrush(RGB(36, 36, 36));

	switch (nCtlColor)
	{
	case CTLCOLOR_STATIC:
		// Labels, texto de grupo, checkboxes, radio buttons — branco suave
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(210, 210, 210));
		return hBrushDark;

	case CTLCOLOR_BTN:
		// Checkboxes e radio buttons
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(210, 210, 210));
		return hBrushDark;

	case CTLCOLOR_EDIT:
		// Campos de texto (CEdit)
		pDC->SetBkColor(RGB(36, 36, 36));
		pDC->SetTextColor(RGB(210, 210, 210));
		return hBrushControl;

	case CTLCOLOR_LISTBOX:
		// Dropdown aberto do ComboBox
		pDC->SetBkColor(RGB(36, 36, 36));
		pDC->SetTextColor(RGB(210, 210, 210));
		return hBrushControl;

	default:
		pDC->SetBkColor(RGB(28, 28, 28));
		pDC->SetTextColor(RGB(210, 210, 210));
		return hBrushDark;
	}
}

BOOL ShortcutsDlg::OnEraseBkgnd(CDC *pDC)
{
	CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(&rect, RGB(28, 28, 28));
	return TRUE;
}

void ShortcutsDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS)
{
	HDC hdc = lpDIS->hDC;
	RECT rc = lpDIS->rcItem;
	UINT state = lpDIS->itemState;

	// ── Push Button owner-draw ────────────────────────────────────────────────
	if (lpDIS->CtlType == ODT_BUTTON)
	{
		bool pressed = (state & ODS_SELECTED) != 0;
		bool focused = (state & ODS_FOCUS) != 0;
		bool disabled = (state & ODS_DISABLED) != 0;

		COLORREF clrBg = pressed ? RGB(120, 0, 60) : disabled ? RGB(28, 28, 28)
															  : RGB(36, 36, 36);
		COLORREF clrBord = focused ? RGB(180, 20, 90) : pressed ? RGB(180, 20, 90)
																: RGB(80, 80, 80);
		COLORREF clrText = pressed ? RGB(255, 20, 147) : disabled ? RGB(80, 80, 80)
																  : RGB(210, 210, 210);

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

		// Texto
		TCHAR szText[128] = {0};
		::GetWindowText(lpDIS->hwndItem, szText, 127);
		::SetBkMode(hdc, TRANSPARENT);
		::SetTextColor(hdc, clrText);
		HFONT hFont = (HFONT)::SendMessage(lpDIS->hwndItem, WM_GETFONT, 0, 0);
		HFONT hOldFont = (HFONT)::SelectObject(hdc, hFont);
		::DrawText(hdc, szText, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		::SelectObject(hdc, hOldFont);
		return;
	}

	// ── ComboBox owner-draw ───────────────────────────────────────────────────
	if (lpDIS->CtlType == ODT_COMBOBOX)
	{
		COLORREF clrBg = (state & ODS_SELECTED) ? RGB(80, 0, 40) : RGB(36, 36, 36);
		COLORREF clrText = (state & ODS_SELECTED) ? RGB(255, 180, 210) : RGB(210, 210, 210);

		HBRUSH hBrush = ::CreateSolidBrush(clrBg);
		::FillRect(hdc, &rc, hBrush);
		::DeleteObject(hBrush);

		if (lpDIS->itemID != (UINT)-1)
		{
			TCHAR szText[256] = {0};
			CComboBox *pCombo = (CComboBox *)GetDlgItem(nIDCtl);
			if (pCombo)
				pCombo->GetLBText(lpDIS->itemID, szText);

			::SetBkMode(hdc, TRANSPARENT);
			::SetTextColor(hdc, clrText);
			RECT rcText = rc;
			rcText.left += 4;
			HFONT hFont = (HFONT)::SendMessage(lpDIS->hwndItem, WM_GETFONT, 0, 0);
			HFONT hOldFont = (HFONT)::SelectObject(hdc, hFont);
			::DrawText(hdc, szText, -1, &rcText,
					   DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
			::SelectObject(hdc, hOldFont);
		}

		if (state & ODS_FOCUS)
		{
			RECT rcFocus = rc;
			::InflateRect(&rcFocus, -1, -1);
			HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(180, 20, 90));
			HPEN hOld = (HPEN)::SelectObject(hdc, hPen);
			::SelectObject(hdc, ::GetStockObject(HOLLOW_BRUSH));
			::Rectangle(hdc, rcFocus.left, rcFocus.top,
						rcFocus.right, rcFocus.bottom);
			::SelectObject(hdc, hOld);
			::DeleteObject(hPen);
		}
		return;
	}

	CDialog::OnDrawItem(nIDCtl, lpDIS);
}

// ─────────────────────────────────────────────────────────────────────────────
