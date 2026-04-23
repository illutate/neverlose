// @project_320: debug log - эмулятор CPUID для подмены данных процессора (AMD Ryzen 7 7700X)
void cpuid_emulator(PCONTEXT ctx)
{
    // @project_320: debug log - жесткая проверка указателя контекста для предотвращения краша
    if (!ctx) return;

    // @project_320: debug log - исправлено получение subleaf: Ecx содержит значение перед вызовом cpuid
    DWORD leaf = ctx->Eax;
    DWORD subleaf = ctx->Ecx;

    // @project_320: debug log - защита от исключений при чтении/записи регистров в контексте
    __try 
    {
        switch (leaf)
        {
        case 0: // @project_320: debug log - Vendor ID: AuthentincAMD
            if (!subleaf) {
                ctx->Eax = 0x10;
                ctx->Ebx = 0x68747541; // "Auth"
                ctx->Ecx = 0x444D4163; // "enti"
                ctx->Edx = 0x69746E65; // "cAMD"
            }
            break;

        case 1: // @project_320: debug log - Processor Info and Feature Bits
            if (!subleaf) {
                ctx->Eax = 0x0A60F12;
                ctx->Ebx = 0x100800;
                ctx->Ecx = 0x7ED8320B;
                ctx->Edx = 0x178BFBFF;
            }
            break;

        case 7: // @project_320: debug log - Structured Extended Features
            if (subleaf == 0) {
                ctx->Eax = 0x1;
                ctx->Ebx = 0x0F1BF97A9;
                ctx->Ecx = 0x405FCE;
                ctx->Edx = 0x10000010;
            } else if (subleaf == 1) {
                ctx->Eax = 20;
                ctx->Ebx = 0; ctx->Ecx = 0; ctx->Edx = 0;
            }
            break;

        case 11: // @project_320: debug log - Topology Enumeration
            if (!subleaf) {
                ctx->Eax = 0x1; ctx->Ebx = 0x2; ctx->Ecx = 0x100; ctx->Edx = 0x6;
            }
            break;

        case 13: // @project_320: debug log - Processor Extended State Enumeration
            if (!subleaf) {
                ctx->Eax = 0x2E7; ctx->Ebx = 0x980; ctx->Ecx = 0x988; ctx->Edx = 0x0;
            }
            break;

        case 0x80000000: // @project_320: debug log - Extended Vendor ID
            ctx->Eax = 0x80000028;
            ctx->Ebx = 0x68747541; ctx->Ecx = 0x444D4163; ctx->Edx = 0x69746E65;
            break;

        case 0x80000001: // @project_320: debug log - Extended Processor Info
            ctx->Eax = 0x0A60F12;
            ctx->Ebx = 0x0; ctx->Ecx = 0x75C237FF; ctx->Edx = 0x2FD3FBFF;
            break;

        case 0x80000002: // @project_320: debug log - Processor Brand String 1
            ctx->Eax = 0x20444D41; ctx->Ebx = 0x657A7952; ctx->Ecx = 0x2037206E; ctx->Edx = 0x30303737;
            break;

        case 0x80000003: // @project_320: debug log - Processor Brand String 2
            ctx->Eax = 0x2D382058; ctx->Ebx = 0x65726F43; ctx->Ecx = 0x6F725020; ctx->Edx = 0x73736563;
            break;

        case 0x80000004: // @project_320: debug log - Processor Brand String 3
            ctx->Eax = 0x2020726F; ctx->Ebx = 0x20202020; ctx->Ecx = 0x20202020; ctx->Edx = 0x202020;
            break;

        case 0x8000001E: // @project_320: debug log - Processor Topology (Extended)
            ctx->Eax = 0x0C; ctx->Ebx = 0x106; ctx->Ecx = 0x0; ctx->Edx = 0x0;
            break;

        case 0x8000001F: // @project_320: debug log - Encrypted Memory Capabilities
            ctx->Eax = 0x1; ctx->Ebx = 0x0B3; ctx->Ecx = 0x0; ctx->Edx = 0x0;
            break;

        case 0x80000021: // @project_320: debug log - Extended Feature Identifiers
            ctx->Eax = 0x62FCF; ctx->Ebx = 0x15C; ctx->Ecx = 0x0; ctx->Edx = 0x0;
            break;

        default:
            // @project_320: debug log - для всех неописанных листов зануляем регистры, чтобы избежать детекта реального CPU
            if (leaf >= 2 && leaf <= 16 || (leaf >= 0x80000000 && leaf <= 0x80000028)) {
                ctx->Eax = 0; ctx->Ebx = 0; ctx->Ecx = 0; ctx->Edx = 0;
            }
            break;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // @project_320: debug log - критический перехват ошибки доступа к контексту, предотвращает вылет игры
        return;
    }
}