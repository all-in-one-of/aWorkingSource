// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FN_KATANA_DISPLAY_DRIVER_H
#define FN_KATANA_DISPLAY_DRIVER_H

/**
 * @defgroup DD Display Driver API
 * @{
 *
 * @brief The Display Driver API allows you to write display drivers that send
 * data from your render process back to the Katana Catalog.
 */


/*
 * Message Definitions for on-the-wire format and base Message class that
 * handles low level message sending.
 */
#include "Message.h"

/*
 * Interface to the client side of communication channel & utility singleton
 */
#include "KatanaPipe.h"
#include "KatanaPipeSingleton.h"

/*
 * Wrappers around on-the-wire messages.
 */
#include "NewFrameMessage.h"
#include "NewChannelMessage.h"
#include "DataMessage.h"
#include "CloseMessage.h"

/*
 * Display Driver host API.
 */
#include "FnDisplayDriver.h"

/**
 * @}
 */
#endif
