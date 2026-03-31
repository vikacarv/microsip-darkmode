#include "stdafx.h"
#include "DarkTabCtrl.h"

BEGIN_MESSAGE_MAP(CDarkTabCtrl, CTabCtrl)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CDarkTabCtrl::CDarkTabCtrl() {}
CDarkTabCtrl::~CDarkTabCtrl() {}

BOOL CDarkTabCtrl::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);
    pDC->FillSolidRect(&rect, RGB(28, 28, 28));
    return TRUE;
}

void CDarkTabCtrl::DrawTab(CDC* pDC, int nTab, BOOL bSelected)
{
    CRect rect;
    GetItemRect(nTab, &rect);

    TCHAR szLabel[256];
    TC_ITEM tci;
    tci.mask = TCIF_TEXT;
    tci.pszText = szLabel;
    tci.cchTextMax = 255;
    GetItem(nTab, &tci);

    if (bSelected) {
        pDC->FillSolidRect(&rect, RGB(28, 28, 28));
        CPen pen(PS_SOLID, 2, RGB(180, 20, 90));
        CPen* pOldPen = pDC->SelectObject(&pen);
        pDC->MoveTo(rect.left, rect.bottom - 1);
        pDC->LineTo(rect.right, rect.bottom - 1);
        pDC->SelectObject(pOldPen);
        pDC->SetTextColor(RGB(180, 20, 90));
    }
    else {
        pDC->FillSolidRect(&rect, RGB(28, 28, 28));
        pDC->SetTextColor(RGB(120, 120, 120));
    }

    pDC->SetBkMode(TRANSPARENT);
    pDC->DrawText(szLabel, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CDarkTabCtrl::OnPaint()
{
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);
    dc.FillSolidRect(&rect, RGB(28, 28, 28));

    int nCount = GetItemCount();
    int nSel = GetCurSel();
    for (int i = 0; i < nCount; i++) {
        DrawTab(&dc, i, i == nSel);
    }
}