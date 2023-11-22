#pragma once

void PSInit(void);
void PSDispose(void);

void PSWrite(BYTE* pbyMessage);
int PSRead(BYTE* pbyMessage);