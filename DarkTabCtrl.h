#pragma once
#include <afxcmn.h>

class CDarkTabCtrl : public CTabCtrl
{
public:
    CDarkTabCtrl();
    virtual ~CDarkTabCtrl();
    BOOL SubclassDlgItem(UINT nID, CWnd* pParent);

protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    DECLARE_MESSAGE_MAP()

private:
    void DrawTab(CDC* pDC, int nTab, BOOL bSelected);
};