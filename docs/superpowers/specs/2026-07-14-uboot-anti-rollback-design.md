# U-Boot Upgrade Version Check Design

## Goal

Prevent repeatedly installing the exact same upgrade image while allowing
switching to any other valid firmware timestamp. For example, a device at
`20260715000000` may install `20260714000000`, but must reject
`20260715000000`.

## Version Source

The upgrade image version is the 14-digit timestamp stored in the image header
as `#<upgrade_bin_version=YYYYMMDDhhmmss>`. The installed version is read from
the U-Boot environment variable `upgrade_image_version`.

Existing devices may only contain the historical misspelling
`uprade_image_version`. The check must fall back to that variable when the
correct variable is absent.

## Validation And Comparison

- An image version must contain exactly 14 decimal digits; otherwise reject it.
- If the installed version is a valid 14-digit value, permit the upgrade when
  the image version is different.
- Reject only equal versions.
- If neither the correct nor legacy environment variable exists, treat the
  operation as the device's first versioned upgrade and allow it.
- After a successful upgrade, existing code stores the image version in
  `upgrade_image_version`, completing migration from the legacy spelling.

Because the product now allows returning to older firmware, the check only uses
the fixed `YYYYMMDDhhmmss` format for validation and equality comparison.

## Scope

Only the U-Boot SD upgrade version gate in
`AK37E_SDK_V1.05/os/uboot/common/cmd_sd_upgrade.c` is changed. Image generation
continues using the existing timestamp in
`AK37E_SDK_V1.05/upgrade/make_image.sh`.

## Verification

Verify the comparison cases for older, equal, newer, invalid, missing-current,
and legacy-variable versions. Older and newer image versions should both be
accepted; equal versions should be rejected. Then rebuild U-Boot and confirm
`AK37E_SDK_V1.05/os/ubd/u-boot.bin` is generated.
