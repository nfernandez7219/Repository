/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_test.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_test.h"
#include "config/default/peripheral/nvmctrl/plib_nvmctrl.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_TEST_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_TEST_DATA app_testData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_TEST_Initialize ( void )

  Remarks:
    See prototype in app_test.h.
 */

void APP_TEST_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_testData.state = APP_TEST_STATE_INIT;



    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

static volatile uint16_t nvmctrlStatus = 0;

/******************************************************************************
  Function:
    void APP_TEST_Tasks ( void )

  Remarks:
    See prototype in app_test.h.
 */

void APP_TEST_Tasks ( void )
{
    int bank_a = 0;

    /* Check the application's current state. */
    switch ( app_testData.state )
    {
        /* Application's initial state. */
        case APP_TEST_STATE_INIT:
        {
            bool appInitialized = true;

            nvmctrlStatus  = NVMCTRL_StatusGet();

            if ((nvmctrlStatus & (NVMCTRL_STATUS_AFIRST_Msk)) != 0)
            {
                    bank_a = 1;
            }
            
            /* try printing some text to the serial line... */
            printf("\n\rThe test application running from NVM Flash BANK %s\n\r", bank_a ? "A": "B");
            
            if (appInitialized)
            {
                app_testData.state = APP_TEST_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_TEST_STATE_SERVICE_TASKS:
        {

            break;
        }

        default:
        {
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
