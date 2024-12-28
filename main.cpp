#include <windows.h>
#include "nya_commonhooklib.h"

#include "fo2.h"
#include "../nya-common-fouc/fo2versioncheck.h"

// fo1 code
void ApplyDamageValues(Suspension* pSuspension, float bumpDamp, float reboundDamp) {
	float damageDelta = 1.0 - ((1.0 - pSuspension->fHealth) * 0.9);
	pSuspension->fBumpDamp = damageDelta * bumpDamp;
	pSuspension->fReboundDamp = damageDelta * reboundDamp;
}

void ApplySuspensionDamage(Car* car) {
	ApplyDamageValues(&car->aSuspension[0], car->fSuspensionFrontBumpDamp,car-> fSuspensionFrontReboundDamp);
	ApplyDamageValues(&car->aSuspension[1], car->fSuspensionFrontBumpDamp,car-> fSuspensionFrontReboundDamp);
	ApplyDamageValues(&car->aSuspension[2], car->fSuspensionRearBumpDamp, car->fSuspensionRearReboundDamp);
	ApplyDamageValues(&car->aSuspension[3], car->fSuspensionRearBumpDamp, car->fSuspensionRearReboundDamp);
}

void __fastcall FixSuspensionDamage(Car* car) {
	for (int i = 0; i < 4; i++) {
		car->aSuspension[i].fHealth = 1;
	}
	ApplySuspensionDamage(car);
}

void __stdcall CalculateSuspensionDamage(Car* pCar, uint32_t damageValue, int nTire) {
	pCar->aSuspension[nTire].fHealth = *(float*)&damageValue;
	ApplySuspensionDamage(pCar);
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
			DoFlatOutVersionCheck(FO2Version::FO2_1_2);

			NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x427080, &SuspensionDamageASM);
			NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x4277AB, &FixSuspensionDamageASM);
		} break;
		default:
			break;
	}
	return TRUE;
}