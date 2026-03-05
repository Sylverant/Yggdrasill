/*
	This file is part of Yggdrasill
	Copyright (C) 2012, 2013, 2026 Lawrence Sebald

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 3 as
	published by  the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pch.h"
#include "Yggdrasill.h"
#include "afxdialogex.h"
#include "SNAKDialog.h"

#include <cinttypes>

static const BYTE char_map[] = {
    0x4a, 0x43, 0x0a, 0x13, 0x5e, 0x6f, 0x58, 0x5b,
    0x46, 0x18, 0x25, 0x51, 0x60, 0x15, 0x7d, 0x64,
    0x0b, 0x71, 0x0d, 0x1e, 0x7c, 0x27, 0x43, 0x7e,
    0x10, 0x2c, 0x4f, 0x15, 0x31, 0x32, 0x04, 0x40,
    0x51, 0x21, 0x4d, 0x63, 0x6b, 0x4a, 0x6e, 0x7e,
    0x62, 0x56, 0x49, 0x16, 0x1c, 0x07, 0x1f, 0x01,
    0x16, 0x03, 0x5c, 0x72, 0x0b, 0x06, 0x30, 0x0a,
    0x72, 0x69, 0x46, 0x7b, 0x04, 0x0e, 0x6d, 0x48
};

static LSTATUS OpenPSORegKey(HKEY &key) {
    return RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\SonicTeam\\PSOV2"), 0,
        KEY_READ | KEY_WRITE, &key);
}

static LSTATUS GetSerial(HKEY regKey, tstring &out) {
	BYTE buf[8];
	DWORD sz = sizeof(buf);
	LSTATUS err;
	int i;
	uint32_t tmp;
	char serial[11] = { 0 };

	err = RegQueryValueEx(regKey, _T("SERIAL"), 0, NULL, (LPBYTE)buf, &sz);
	if(err == ERROR_SUCCESS) {
		// Grab the hex string first
		for(i = 0; i < 8; ++i) {
			serial[i] = buf[i] ^ char_map[i];
		}

		// Convert to a decimal string....
		tmp = (uint32_t)std::strtoul(serial, NULL, 16);
#ifdef UNICODE
		out = std::to_wstring(tmp);
#else
		out = std::to_string(tmp);
#endif
	}

	return err;
}

static LSTATUS SetSerial(HKEY regKey, CStringA &in) {
	BYTE buf[8];
	DWORD sz = sizeof(buf);
	unsigned long long tmp;
	char serial[9];
	int i;

	tmp = std::strtoull(in.GetString(), NULL, 10);
	if(tmp > 0xFFFFFFFF)
		return ERROR_APP_DATA_CORRUPT;

	sprintf_s(serial, 9, "%08" PRIX32, (uint32_t)tmp);

	for(i = 0; i < 8; ++i) {
		buf[i] = serial[i] ^ char_map[i];
	}

	return RegSetValueEx(regKey, _T("SERIAL"), 0, REG_BINARY, buf, sz);
}

static LSTATUS GetAccessKey(HKEY regKey, tstring &out) {
	BYTE buf[8];
	DWORD sz = sizeof(buf);
	LSTATUS err;
	int i;
	char access[9] = { 0 };

	err = RegQueryValueEx(regKey, _T("ACCESS"), 0, NULL, (LPBYTE)buf, &sz);
	if(err == ERROR_SUCCESS) {
		// Decrypt it
		for(i = 0; i < 8; ++i) {
			access[i] = buf[i] ^ char_map[i];
		}

		CString tmp(access);
		out = tmp;
	}

	return err;
}

static LSTATUS SetAccessKey(HKEY regKey, CStringA &in) {
	BYTE buf[8];
	DWORD sz = sizeof(buf);
	int i;

	for(i = 0; i < 8; ++i) {
		buf[i] = in[i] ^ char_map[i];
	}

	return RegSetValueEx(regKey, _T("ACCESS"), 0, REG_BINARY, buf, sz);
}

static LSTATUS GetEMail(HKEY regKey, tstring &out) {
	BYTE buf[64];
	DWORD sz = sizeof(buf);
	LSTATUS err;
	int i;
	char email[65] = { 0 };

	err = RegQueryValueEx(regKey, _T("E-MAIL"), 0, NULL, (LPBYTE)buf, &sz);
	if(err == ERROR_SUCCESS) {
		// Decrypt it
		for(i = 0; i < 64; ++i) {
			email[i] = buf[i] ^ char_map[i];
		}

		CString tmp(email);
		out = tmp;
	}

	return err;
}

static LSTATUS SetEMail(HKEY regKey, CStringA &in) {
	BYTE buf[64];
	DWORD sz = sizeof(buf);
	char serial[9];
	int i;

	for(i = 0; i < in.GetLength(); ++i) {
		buf[i] = in[i] ^ char_map[i];
	}

	for(; i < 64; ++i) {
		buf[i] = char_map[i];
	}

	return RegSetValueEx(regKey, _T("E-MAIL"), 0, REG_BINARY, buf, sz);
}


IMPLEMENT_DYNAMIC(SNAKDialog, CDialog)

SNAKDialog::SNAKDialog(CWnd *pParent)
	: CDialog(IDD_SNAK_DIALOG, pParent) {
}

SNAKDialog::~SNAKDialog() {
}

void SNAKDialog::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SNAK_ACCESS, m_accessKey);
	DDX_Control(pDX, IDC_SNAK_EMAIL, m_email);
	DDX_Control(pDX, IDC_SNAK_SERIAL, m_serial);
}

BEGIN_MESSAGE_MAP(SNAKDialog, CDialog)
	ON_BN_CLICKED(IDOK, &SNAKDialog::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL SNAKDialog::OnInitDialog() {
	CDialog::OnInitDialog();

	tstring serial, access, email;
	HKEY key;
	LSTATUS err;

	err = OpenPSORegKey(key);

	if(err == 0) {
		GetSerial(key, serial);
		GetAccessKey(key, access);
		GetEMail(key, email);
		RegCloseKey(key);
	}
	else {
		MessageBox(_T("Could not read PSO registry key!"));
		CDialog::OnCancel();
		return TRUE;
	}

	m_serial.SetWindowText(serial.c_str());
	m_accessKey.SetWindowText(access.c_str());
	m_email.SetWindowText(email.c_str());

	return TRUE;
}

void SNAKDialog::OnBnClickedOk() {
	CString serial, access, email;
	char *loc;
	int i;

	m_serial.GetWindowText(serial);
	m_accessKey.GetWindowText(access);
	m_email.GetWindowText(email);

	if(serial.GetLength() > 10) {
		MessageBox(_T("Invalid Serial Number"));
		return;
	}

	if(access.GetLength() != 8) {
		MessageBox(_T("Invalid Access Key"));
		return;
	}

	if(email.GetLength() < 5 || email.GetLength() > 64) {
		MessageBox(_T("Invalid E-Mail Address"));
		return;
	}

	/* Check the format of the serial/access key */
	loc = std::setlocale(LC_ALL, nullptr);
	std::setlocale(LC_ALL, "C");
	CStringA ser(serial);
	CStringA acc(access);
	CStringA eml(email);

	for(i = 0; i < ser.GetLength(); ++i) {
		if(!std::isdigit(ser[i])) {
			MessageBox(_T("Invalid Serial Number"));
			return;
		}
	}

	for(i = 0; i < acc.GetLength(); ++i) {
		if(!std::isalnum(acc[i])) {
			MessageBox(_T("Invalid Access Key"));
			return;
		}
	}

	std::setlocale(LC_ALL, loc);

	HKEY key;
	LSTATUS err;

	OpenPSORegKey(key);

	err = SetSerial(key, ser);
	if(err == ERROR_APP_DATA_CORRUPT) {
		MessageBox(_T("Invalid Serial Number"));
		return;
	}
	else if(err != 0) {
		MessageBox(_T("Could not write serial number to registry!"));
		return;
	}

	err = SetAccessKey(key, acc);
	if(err == ERROR_APP_DATA_CORRUPT) {
		MessageBox(_T("Invalid Access Key"));
		return;
	}
	else if(err != 0) {
		MessageBox(_T("Could not write access key to registry!"));
		return;
	}

	err = SetEMail(key, eml);
	if(err == ERROR_APP_DATA_CORRUPT) {
		MessageBox(_T("Invalid E-Mail Address"));
		return;
	}
	else if(err != 0) {
		MessageBox(_T("Could not write e-mail address to registry!"));
		return;
	}

	RegCloseKey(key);

	CDialog::OnOK();
}
