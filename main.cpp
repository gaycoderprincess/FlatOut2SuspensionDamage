#include <windows.h>
#include <fstream>
#include "nya_commonhooklib.h"

class Suspension {
public:
	float fBumpDamp; // +0
	float fReboundDamp; // +4
	uint8_t _8[0x24];
	float fHealth; // +2C
	uint8_t _30[0x10];

	void ApplyDamageValues(float bumpDamp, float reboundDamp) {
		float damageDelta = 1.0 - ((1.0 - fHealth) * 0.9);
		fBumpDamp = damageDelta * bumpDamp;
		fReboundDamp = damageDelta * reboundDamp;
	}
};

class Car {
public:
	uint8_t _0[0x1890];
	Suspension aSuspension[4]; // +1890
	uint8_t _1990[0x3A8];
	float fSuspensionFrontBumpDamp; // +1D38
	float fSuspensionFrontReboundDamp; // +1D3C
	uint8_t _1D40[0x3C];
	float fSuspensionRearBumpDamp; // +1D7C
	float fSuspensionRearReboundDamp; // +1D80

	void ApplySuspensionDamage() {
		aSuspension[0].ApplyDamageValues(fSuspensionFrontBumpDamp, fSuspensionFrontReboundDamp);
		aSuspension[1].ApplyDamageValues(fSuspensionFrontBumpDamp, fSuspensionFrontReboundDamp);
		aSuspension[2].ApplyDamageValues(fSuspensionRearBumpDamp, fSuspensionRearReboundDamp);
		aSuspension[3].ApplyDamageValues(fSuspensionRearBumpDamp, fSuspensionRearReboundDamp);
	}

	void FixSuspensionDamage() {
		for (int i = 0; i < 4; i++) {
			aSuspension[i].fHealth = 1;
		}
		ApplySuspensionDamage();
	}
};

void __stdcall CalculateSuspensionDamage(Car* pCar, uint32_t damageValue, int nTire) {
	pCar->aSuspension[nTire].fHealth = *(float*)&damageValue;
	pCar->ApplySuspensionDamage();
}

uintptr_t SuspensionDamageASM_jmp = 0x42708A;
void __attribute__((naked)) SuspensionDamageASM() {
	__asm__ (
		"mov eax, [esp+0x20]\n\t"
		"pushad\n\t"
		"push edi\n\t" // nTire
		"push eax\n\t" // damageValue
		"push ebp\n\t" // pCar
		"call %1\n\t"
		"popad\n\t"
		"mov [esi+0x2C4], eax\n\t"
		"jmp %0\n\t"
			:
			:  "m" (SuspensionDamageASM_jmp), "i" (CalculateSuspensionDamage)
	);
}

void __fastcall FixSuspensionDamage(Car* pCar) {
	pCar->FixSuspensionDamage();
}

void __attribute__((naked)) FixSuspensionDamageASM() {
	__asm__ (
		"pushad\n\t"
		"mov ecx, edi\n\t"
		"call %0\n\t"
		"popad\n\t"
		"mov dword ptr [edi+0x1C0C], 0\n"
		"pop edi\n"
		"ret\n\t"
			:
			:  "i" (FixSuspensionDamage)
	);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x202638) {
				MessageBoxA(nullptr, "Unsupported game version! Make sure you're using DRM-free v1.2 (.exe size of 2990080 bytes)", "nya?!~", MB_ICONERROR);
				exit(0);
				return TRUE;
			}

			NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x427080, &SuspensionDamageASM);
			NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x4277AB, &FixSuspensionDamageASM);
		} break;
		default:
			break;
	}
	return TRUE;
}