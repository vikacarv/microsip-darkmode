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
#include "StatusBar.h"
#include "global.h"

 // StatusBar
IMPLEMENT_DYNAMIC(StatusBar, CStatusBar)
StatusBar::StatusBar()
{
	CStatusBar::CStatusBar();
}

StatusBar::~StatusBar()
{
}

BEGIN_MESSAGE_MAP(StatusBar, CStatusBar)
	//{{AFX_MSG_MAP(StatusBar)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

LRESULT StatusBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	if (IsWindowVisible())
	{
		CFrameWnd* pParent = (CFrameWnd*)GetParent();
		if (pParent)
			OnUpdateCmdUI(pParent, (BOOL)wParam);
	}
	return 0L;
}

void StatusBar::OnLButtonUp(UINT nFlags, CPoint point)
{
}

void StatusBar::OnMouseMove(UINT nFlags, CPoint point)
{
}
// ===== DARK MODE =====
BOOL StatusBar::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);
    pDC->FillSolidRect(&rect, RGB(20, 20, 20));
    return TRUE;
}

void StatusBar::OnPaint()
{
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);
    dc.FillSolidRect(&rect, RGB(20, 20, 20));
    dc.SetTextColor(RGB(180, 20, 90));
    dc.SetBkMode(TRANSPARENT);

    int nCount = GetCount();
    for (int i = 0; i < nCount; i++) {
        CRect paneRect;
        GetItemRect(i, &paneRect);
        CString text;
        UINT nID, nStyle;
        int nWidth;
        GetPaneInfo(i, nID, nStyle, nWidth);
        GetPaneText(i, text);
        if (i == 0) {
            // ícone
            HICON hIcon = GetStatusBarCtrl().GetIcon(i);
            if (hIcon) {
                ::DrawIconEx(dc.m_hDC, paneRect.left + 2,
                    paneRect.top + (paneRect.Height() - 16) / 2,
                    hIcon, 16, 16, 0, NULL, DI_NORMAL);
                paneRect.left += 20;
            }
        }
        dc.DrawText(text, &paneRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }
}
// ===== FIM DARK MODE =====
