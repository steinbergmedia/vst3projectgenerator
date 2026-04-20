#include <LicenseSpring/Configuration.h>
#include <LicenseSpring/EncryptString.h>
#include <LicenseSpring/LicenseManager.h>
#include <LicenseSpring/Exceptions.h>
#include <iostream>
#include <thread>

using namespace LicenseSpring;

//Uses check() and registerFloatingLicense() to continuously refresh the timeout interval.
void check_reg( License::ptr_t license );

//Uses a watchdog to run checks() on a background thread, in intervals, thus continuously refreshing the timeout interval.
void watchdog( License::ptr_t license );

//This sample code will go through how a floating license, using the LicenseSpring servers' cloud, can be registered,
//released/deregistered, timed-out, and renewed. When testing this sample code, it is recommended to set 
//floating timeout to a small value such as 1 minute, to be able to see the timeout feature.
int main()
{

    std::string appName = "NAME"; //input name of application
    std::string appVersion = "VERSION"; //input version of application

    //Collecting network info
    ExtendedOptions options;
    options.collectNetworkInfo( true );
    options.enableLogging( true );

    std::shared_ptr<Configuration> pConfiguration = Configuration::Create(
        EncryptStr( "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX" ), // your LicenseSpring API key (UUID)
        EncryptStr( "XXXXXXXXX-XXXXX-XXXXXXXXXXXXX_XXXXXX_XXXXXX" ), // your LicenseSpring Shared key
        EncryptStr( "XXXXXX" ), // product code that you specified in LicenseSpring for your application
        appName, appVersion, options );

    //Key-based implementation
    auto licenseId = LicenseID::fromKey( "XXXX-XXXX-XXXX-XXXX" ); //input license key

    //For user-based implementation comment out above line, and use bottom 3 lines
 //   const std::string userId = "example@email.com"; //input user email
 //   const std::string userPassword = "password"; //input user password
 //   auto licenseId = LicenseID::fromUser( userId, userPassword );

    std::shared_ptr<LicenseManager> licenseManager = LicenseManager::create( pConfiguration );

    License::ptr_t license = nullptr;

    //Find our local license if we have one stored on our device.
    try
    {
        license = licenseManager->reloadLicense();
    }
    catch ( LocalLicenseException )
    {  //Exception if we cannot read the local license or the local license file is corrupt
        std::cout << "Could not read previous local license. Local license may be corrupt. "
            << "Create a new local license by activating your license." << std::endl;
        return 0;
    }

    //We'll activate our license if we haven't already done so.
    try
    {
        if ( license == nullptr )
            license = licenseManager->activateLicense(licenseId);

        //We'll do a local check right after just to make sure everything is working properly.
        license->localCheck();
    }
    //Possible LicenseSpring exceptions, we won't pay too much attention on this tutorial and assume
    //everything is working properly for the user.
    catch ( LicenseSpringException ex )
    {
        std::cout << ex.what() << std::endl;
        return 0;
    }

    //By default, auto release on license is set to true. When it is true, the moment the License Destrutor is called, 
    //your floating license will be released. That means when your program ends or if you destroy the License object, 
    //with something like clearLocalStorage(), the floating license will be released. By setting it to false, 
    //the floating license will only be released, either by running releaseFloatingLicense, or if it times out.
    //Uncomment out this line if you want to test that.
    //license->setAutoRelease( false );

    //We can use this to check if our license has floating options allowed.
    if ( license->isFloating() )
    {
        try
        {
            //We'll try and register our floating license here. Note, we get an exception if there are no more floating
            //licenses available. You can also use license->check(), as it will also register your floating license,
            //and will throw the same exception if there are no available floating licenses.
            license->registerFloatingLicense();
            //license->check()
        }
        catch ( MaxFloatingReachedException )
        {
            std::cout << "No more available floating licenses." << std::endl;
            return 0;
        }

        std::cout << "Currently there are: " << license->floatingInUseCount() << " floating licenses, with: "
            << license->maxFloatingUsers() - license->floatingInUseCount() << " floating licenses available." << std::endl;

        std::cout << "Your floating license has a timeout period of: " << license->floatingTimeout()
            << " minute." << std::endl;

        //If a user wants to stay signed in on their floating license, regardless of their floating timeout, they'll have to 
        //re-register their license, which will reset their timeout clock. There are 3 ways to do this.

        //Method 1/Method 2:
        //Use either registerFloatingLicense() or check() to reset the timer. Both of these functions will reset your
        //floating license timeout, and extend your period using the license.

        check_reg( license ); //see function below
        
        //Method 3:
        //Using a licenseWatchdog. This will setup a background thread which will periodically run an online check
        //thus refreshing the timeout interval. This method is particularly useful for floating cloud licenses.

        //watchdog( license ) //see function below
    }
    else
    {
        std::cout << "This sample code is only used for testing floating cloud licenses. "
            << "If you would like to test another license feature, see our other samples." << std::endl;
        return 0;
    }

    //If you set auto release to false, then it is recommended that you release the floating license at the end of the
    //application/program, or else that device will hold onto the floating license until it timesout, or they renew it.
    license->releaseFloatingLicense();

    return 0;
}

//This is just a function that runs continuously, asking the user to check/register before they are timed out.
void check_reg( License::ptr_t license )
{
    std::string sInput = "";

    //We'll use this for loop to simulate a running app.
    for ( int i = 0; true; i++ )
    {
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
        std::cout << i << std::endl;

        if ( i == license->floatingTimeout() * 60 - 15 ) //floatingTimeout is in minutes, and we subtract 15 seconds as a buffer
        {                                                //for the user to make a decision and input y
            std::cout << "You're about to be kicked out of your floating cloud license."
                << "If you want to stay in, type 'y'. If you want to leave, type anything else." << std::endl;
            std::getline( std::cin, sInput );
            if ( sInput.compare( "y" ) == 0 )
            {
                //Either of these methods will reset your timeout interval.
                license->registerFloatingLicense();
                //license->check();

                i = -1;
            }
            else
            {
                break;
            }
        }
    }
}

//This is our watchdog function that will set up our watchdog, and then run an infinite loop until the user exits.
//We run the inifinite loop so that the user can confirm they're still using the floating license on the platofrm.
//You can also check the logs to see that the license is being checked every floating timeout interval.
void watchdog( License::ptr_t license )
{
    std::weak_ptr<License> wpLicense( license );
    license->setupLicenseWatchdog( [ wpLicense ]( const LicenseSpringException& ex ) //The first parameter is the callback
        {
            // Attention, do not capture License::ptr_t (shared_ptr), this will lead to problems.

            std::cout << std::endl << "License check failed: " << ex.what() << std::endl;

            if ( ex.getCode() == eMaxFloatingReached )
            {
                std::cout << "Application cannot use this license at the moment because floating license limit reached." << std::endl;
                exit( 0 );
            }

            // Ignore other errors and continue running watchdog if possible
            if ( auto pLicense = wpLicense.lock() )
            {
                if ( pLicense->isValid() )
                    pLicense->resumeLicenseWatchdog();
            }
        }, license->floatingTimeout() ); //The second parameter is the time interval we'll do online checks (in minutes)
    
    std::string sInput = "";
    std::cout << "While watchdog is up, your license should be checked periodically. Check the platform to confirm. "
        << "Type 'e' to exit." << std::endl;
    while ( sInput.compare( "e" ) != 0 )
    {
        std::getline( std::cin, sInput );
    }
}
