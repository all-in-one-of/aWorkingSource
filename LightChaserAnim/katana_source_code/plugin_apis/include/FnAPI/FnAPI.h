// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FoundryKatanaAPI_H
#define FoundryKatanaAPI_H

#define KATANA_TO_STR(_str) _KATANA_STR(_str)
#define _KATANA_STR(_str)   #_str

// The three components of the Katana version
#define KATANA_VERSION_MAJOR   2
#define KATANA_VERSION_MINOR   1
#define KATANA_VERSION_RELEASE 5

// Strings for the three components of the Katana version
#define KATANA_VERSION_STR_MAJOR   KATANA_TO_STR(KATANA_VERSION_MAJOR)
#define KATANA_VERSION_STR_MINOR   KATANA_TO_STR(KATANA_VERSION_MINOR)
#define KATANA_VERSION_STR_RELEASE KATANA_TO_STR(KATANA_VERSION_RELEASE)

// String for the full Katana version
#define KATANA_VERSION_STR  KATANA_VERSION_STR_MAJOR "."\
                            KATANA_VERSION_STR_MINOR "."\
                            KATANA_VERSION_STR_RELEASE

#endif // FoundryKatanaAPI_H

