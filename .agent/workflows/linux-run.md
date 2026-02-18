---
description: Linuxda SimulIDE-ni quraşdırmaq və işlətmək qaydası
---
Bu workflow SimulIDE-ni Linux (Ubuntu/Debian) mühitində necə quraşdırıb işlədəcəyinizi göstərir.

1. **Terminalı açın** və layihə qovluğuna keçin.

2. **Lazımi kitabxanaları quraşdırın**:
   // turbo
   ```bash
   chmod +x scripts/setup-linux.sh
   sudo ./scripts/setup-linux.sh
   ```

3. **Proqramı qurun və başladın**:
   // turbo
   ```bash
   chmod +x scripts/build-linux.sh
   ./scripts/build-linux.sh
   ```

İndi SimulIDE Linux-da işə düşməlidir.
