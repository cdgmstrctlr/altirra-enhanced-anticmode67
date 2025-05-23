#ifndef f_AT_UICOMMONDIALOGS_H
#define f_AT_UICOMMONDIALOGS_H

#include <vd2/system/refcount.h>
#include <vd2/system/VDString.h>
#include "uiqueue.h"

class VDException;

bool ATUIGetNativeDialogMode();
void ATUISetNativeDialogMode(bool enabled);

///////////////////////////////////////////////////////////////////////////

void ATUIShowInfo(VDGUIHandle h, const wchar_t *text);
void ATUIShowWarning(VDGUIHandle h, const wchar_t *text, const wchar_t *caption);
bool ATUIShowWarningConfirm(VDGUIHandle h, const wchar_t *text, const wchar_t *title = nullptr);
void ATUIShowError2(VDGUIHandle h, const wchar_t *text, const wchar_t *title);
void ATUIShowError(VDGUIHandle h, const wchar_t *text);
void ATUIShowError(VDGUIHandle h, const VDException& e);
void ATUIShowError(const VDException& e);

vdrefptr<ATUIFutureWithResult<bool> > ATUIShowAlertWarningConfirm(const wchar_t *text, const wchar_t *title);
vdrefptr<ATUIFutureWithResult<bool> > ATUIShowAlertError(const wchar_t *text, const wchar_t *title);

///////////////////////////////////////////////////////////////////////////

struct ATUIFileDialogResult : public ATUIFuture {
	bool mbAccepted;
	VDStringW mPath;
};

vdrefptr<ATUIFileDialogResult>  ATUIShowOpenFileDialog(uint32 id, const wchar_t *title, const wchar_t *filters);
vdrefptr<ATUIFileDialogResult>  ATUIShowSaveFileDialog(uint32 id, const wchar_t *title, const wchar_t *filters);

#endif
