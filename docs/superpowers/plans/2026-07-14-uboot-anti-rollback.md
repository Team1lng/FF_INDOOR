# U-Boot Upgrade Version Check Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reject equal and malformed upgrade images while allowing any other valid version, including older images, with first-upgrade and legacy environment-variable compatibility.

**Architecture:** Put the format and equality rules in a small U-Boot-compatible header containing file-local helpers, then use those helpers from the existing SD upgrade gate. A host-side C test directly exercises the same helpers before the U-Boot source is changed.

**Tech Stack:** U-Boot C, POSIX shell, host GCC, existing AK37E U-Boot build scripts.

---

### Task 1: Add Version Rule Tests

**Files:**
- Create: `AK37E_SDK_V1.05/os/uboot/common/upgrade_version_check.h`
- Create: `AK37E_SDK_V1.05/os/uboot/test/upgrade_version_check_test.c`

- [ ] **Step 1: Write the failing host test**

Create a test that requires exactly 14 digits and accepts any different version:

```c
#include <assert.h>
#include "../common/upgrade_version_check.h"

int main(void)
{
    assert(upgrade_version_valid("20260714120000"));
    assert(!upgrade_version_valid("20260714"));
    assert(!upgrade_version_valid("2026071412000x"));
    assert(upgrade_version_is_different("20260715120000", "20260714120000"));
    assert(!upgrade_version_is_different("20260714120000", "20260714120000"));
    assert(upgrade_version_is_different("20260713120000", "20260714120000"));
    return 0;
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
cd AK37E_SDK_V1.05/os/uboot
gcc -Wall -Wextra -std=c89 test/upgrade_version_check_test.c -o /tmp/upgrade_version_check_test
```

Expected: compilation fails because `upgrade_version_check.h` and its helpers do not exist.

- [ ] **Step 3: Add the minimal pure helpers**

Implement `upgrade_version_valid()` and `upgrade_version_is_different()` as `static` C89-compatible helpers. Validation checks a non-null string, exactly 14 characters, and decimal digits only. Equality uses `strcmp()` after validation.

- [ ] **Step 4: Run the test to verify it passes**

Run:

```bash
cd AK37E_SDK_V1.05/os/uboot
gcc -Wall -Wextra -std=c89 test/upgrade_version_check_test.c -o /tmp/upgrade_version_check_test
/tmp/upgrade_version_check_test
```

Expected: compilation succeeds and the executable exits with status 0.

### Task 2: Enforce The Upgrade Gate

**Files:**
- Modify: `AK37E_SDK_V1.05/os/uboot/common/cmd_sd_upgrade.c:20-90`

- [ ] **Step 1: Include the shared rules and legacy variable name**

Include `upgrade_version_check.h` and define:

```c
#define ENV_UPGRADEIMAGE_VERSION_LEGACY "uprade_image_version"
```

- [ ] **Step 2: Safely parse the image version**

Reject a header value that is empty, exceeds the destination buffer, or fails `upgrade_version_valid()`. Explicitly terminate the copied value before validation.

- [ ] **Step 3: Select and validate the installed version**

Read `upgrade_image_version` first, then fall back to `uprade_image_version`. If neither exists, permit the first versioned upgrade. If an installed value exists but is malformed, reject the upgrade rather than bypassing rollback protection.

- [ ] **Step 4: Reject only the same image version**

Return success only when `upgrade_version_is_different(image_version, installed_version)` is true. Log whether the image is accepted, equal, or malformed before returning.

- [ ] **Step 5: Re-run the host test**

Run:

```bash
cd AK37E_SDK_V1.05/os/uboot
gcc -Wall -Wextra -std=c89 test/upgrade_version_check_test.c -o /tmp/upgrade_version_check_test
/tmp/upgrade_version_check_test
```

Expected: exit status 0.

### Task 3: Build U-Boot

**Files:**
- Verify: `AK37E_SDK_V1.05/os/ubd/u-boot.bin`

- [ ] **Step 1: Build the target U-Boot**

Run:

```bash
cd AK37E_SDK_V1.05/os/uboot
./uboot_build.sh
```

Expected: the build exits successfully without a new compiler error in `cmd_sd_upgrade.c`.

- [ ] **Step 2: Confirm the output artifact**

Run:

```bash
stat ../ubd/u-boot.bin
```

Expected: `u-boot.bin` exists and has a current modification timestamp.

- [ ] **Step 3: Review the focused diff**

Run:

```bash
git diff -- common/cmd_sd_upgrade.c common/upgrade_version_check.h test/upgrade_version_check_test.c
```

Expected: only version-check changes appear alongside the user's pre-existing `FF.dtb` change.
