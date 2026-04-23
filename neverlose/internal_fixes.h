#ifndef NEVERLOSE_INTERNAL_FIXES_H
#define NEVERLOSE_INTERNAL_FIXES_H

#pragma once

// Оборачиваем в extern "C", если реализации написаны на чистом C или 
// чтобы избежать искажения имен (name mangling) компилятором C++
#ifdef __cplusplus
extern "C" {
#endif

void fix_mem_dispatcher();
void fix_sha256();
void hijack_requestor();

#ifdef __cplusplus
}
#endif

#endif // NEVERLOSE_INTERNAL_FIXES_H