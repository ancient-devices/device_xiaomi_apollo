/*
 * Copyright (C) 2021 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/properties.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#include <string>
#include <vector>

typedef struct variant_info {
    std::string hwc_value;
    std::string sku_value;

    std::string brand;
    std::string device;
    std::string marketname;
    std::string model;
    std::string build_fingerprint;

    bool nfc;
} variant_info_t;

void search_variant(const std::vector<variant_info_t> variants);
void set_variant_props(const variant_info_t variant);

void property_override(std::string prop, std::string value, bool add = true);
void set_ro_build_prop(const std::string &prop, const std::string &value, bool product = false);

std::string fingerprint_to_description(std::string fingerprint);

using android::base::GetProperty;

#define HWC_PROP "ro.boot.hwc"
#define SKU_PROP "ro.boot.product.hardware.sku"

static const variant_info_t apollocn_info = {
    .hwc_value = "CN",
    .sku_value = "",

    .brand = "Redmi",
    .device = "apollo",
    .marketname = "Redmi K30S Ultra",
    .model = "M2007J3SC",
    .build_fingerprint = "Redmi/apollo/apollo:11/RKQ1.200826.002/V12.5.4.0.RJDCNXM:user/release-keys",

    .nfc = true,
};

static const variant_info_t apollo_info = {
    .hwc_value = "",
    .sku_value = "std",

    .brand = "Xiaomi",
    .device = "apollo",
    .marketname = "Mi 10T",
    .model = "M2007J3SY",
    .build_fingerprint = "Xiaomi/apollo_global/apollo:11/RKQ1.200826.002/V12.5.4.0.RJDMIXM:user/release-keys",

    .nfc = true,
};

static const variant_info_t apollopro_info = {
    .hwc_value = "",
    .sku_value = "pro",

    .brand = "Xiaomi",
    .device = "apollo",
    .marketname = "Mi 10T Pro",
    .model = "M2007J3SY",
    .build_fingerprint = "Xiaomi/apollopro_global/apollo:11/RKQ1.200826.002/V12.5.4.0.RJDMIXM:user/release-keys",

    .nfc = true,
};

static const std::vector<variant_info_t> variants = {
    apollocn_info,
    apollo_info,
    apollopro_info,
};

void vendor_load_properties() {
    search_variant(variants);
}

void search_variant(const std::vector<variant_info_t> variants) {
    std::string hwc_value = GetProperty(HWC_PROP, "");
    std::string sku_value = GetProperty(SKU_PROP, "");

    for (const auto& variant : variants) {
        if ((variant.hwc_value == "" || variant.hwc_value == hwc_value) &&
            (variant.sku_value == "" || variant.sku_value == sku_value)) {
            set_variant_props(variant);
            break;
        }
    }
}


void set_variant_props(const variant_info_t variant) {
    set_ro_build_prop("brand", variant.brand, true);
    set_ro_build_prop("device", variant.device, true);
    set_ro_build_prop("marketname", variant.marketname, true);
    set_ro_build_prop("model", variant.model, true);

    set_ro_build_prop("fingerprint", variant.build_fingerprint);
    property_override("ro.bootimage.build.fingerprint", variant.build_fingerprint.c_str());
    property_override("ro.build.description", fingerprint_to_description(variant.build_fingerprint.c_str()));

    if (variant.nfc)
        property_override(SKU_PROP, "nfc");
}

void property_override(std::string prop, std::string value, bool add) {
    auto pi = (prop_info *) __system_property_find(prop.c_str());
    if (pi != nullptr) {
        __system_property_update(pi, value.c_str(), value.length());
    } else if (add) {
        __system_property_add(prop.c_str(), prop.length(), value.c_str(), value.length());
    }
}

std::vector<std::string> ro_props_default_source_order = {
    "odm.",
    "product.",
    "system.",
    "system_ext.",
    "vendor.",
    "",
};

void set_ro_build_prop(const std::string &prop, const std::string &value, bool product) {
    std::string prop_name;

    for (const auto &source : ro_props_default_source_order) {
        if (product)
            prop_name = "ro.product." + source + prop;
        else
            prop_name = "ro." + source + "build." + prop;

        property_override(prop_name, value, true);
    }
}

#define FIND_AND_REMOVE(s, delimiter, variable_name) \
    std::string variable_name = s.substr(0, s.find(delimiter)); \
    s.erase(0, s.find(delimiter) + delimiter.length());

#define APPEND_STRING(s, to_append) \
    s.append(" "); \
    s.append(to_append);

std::string fingerprint_to_description(std::string fingerprint) {
    std::string delimiter = "/";
    std::string delimiter2 = "/";
    std::string build_fingerprint_copy = fingerprint;

    FIND_AND_REMOVE(build_fingerprint_copy, delimiter, brand)
    FIND_AND_REMOVE(build_fingerprint_copy, delimiter, product)
    FIND_AND_REMOVE(build_fingerprint_copy, delimiter2, device)
    FIND_AND_REMOVE(build_fingerprint_copy, delimiter, platform_version)
    FIND_AND_REMOVE(build_fingerprint_copy, delimiter, build_id)
    FIND_AND_REMOVE(build_fingerprint_copy, delimiter2, build_number)
    FIND_AND_REMOVE(build_fingerprint_copy, delimiter, build_variant)
    std::string build_version_tags = build_fingerprint_copy;

    std::string description = product + "-" + build_variant;
    APPEND_STRING(description, platform_version)
    APPEND_STRING(description, build_id)
    APPEND_STRING(description, build_number)
    APPEND_STRING(description, build_version_tags)

    return description;
}