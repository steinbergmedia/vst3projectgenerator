//Remember to include these headers, they may have different paths if you are using this code in a different folder.
#include <LicenseSpring/Configuration.h>
#include <LicenseSpring/EncryptString.h>
#include <LicenseSpring/LicenseManager.h>
#include <LicenseSpring/Exceptions.h>
#include <LicenseSpring/FloatingClient.h>
#include <LicenseSpring/BaseManager.h>
#include <iostream>
#include <thread>
#include <LicenseSpring/InstallationFile.h>

#pragma warning( disable : 4996 )

using namespace LicenseSpring;

//License Checking function at bottom of code. Shows how to do an online check and sync, as well as a local check.
void LicenseCheck( License::ptr_t license );

//Our console Product Version ChatBot program that allows the user to view all available versions for product and receive installation URL.
int main()
{
    std::string appName = "NAME"; //input name of application
    std::string appVersion = "VERSION"; //input version of application

    //Collecting network info
    ExtendedOptions options;
    options.collectNetworkInfo( true );
    options.enableLogging( true );

    std::shared_ptr<Configuration> pConfiguration = Configuration::Create( 
        EncryptStr( "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX" ), // your LicenseSpring API key ( UUID )
        EncryptStr( "XXXXXXXXX-XXXXX-XXXXXXXXXXXXX_XXXXXX_XXXXXX" ), // your LicenseSpring Shared key
        EncryptStr( "XXXXXX" ), // product code that you specified in LicenseSpring for your application
        appName, appVersion, options );

    //Key-based implementation
    auto licenseId = LicenseID::fromKey( "XXXX-XXXX-XXXX-XXXX" ); //input license key

    auto licenseManager = LicenseManager::create( pConfiguration );


    //getCurrentLicense() will return a pointer to the local license stored
    //on the end-user's device if they have one that matches the current 
    //configuration i.e. API key, Shared key, and product code.
    License::ptr_t license = nullptr;
    try
    {
        license = licenseManager->getCurrentLicense();
    }
    catch ( LocalLicenseException )
    { //Exception if we cannot read the local license or the local license file is corrupt
        std::cout << "Could not read previous local license. Local license may be corrupt. "
            << "Create a new local license by activating your license." << std::endl;
        return 0;
    }

    //We'll do a quick license check to make sure everything is fine on our license before we start.
    LicenseCheck( license );

    if ( license == nullptr || !license->isActive() )
    {
        try 
        {
            //Activates license on LicenseSpring servers and creates/updates pointer
            //to local license file. Throws an exception if we are over the max number of activations
            license = licenseManager->activateLicense( licenseId );
        }
        catch ( LicenseNoAvailableActivationsException )
        {
            std::cout << "No available activations." << std::endl;
        }
        catch ( LicenseStateException )
        {
            std::cout << "Current license is not valid" << std::endl;
            if ( license != nullptr && !license->isActive() )
                std::cout << "License is inactive" << std::endl;
            if ( license != nullptr && license->isExpired() )
                std::cout << "License is expired" << std::endl;
            if ( license != nullptr && !license->isEnabled() )
                std::cout << "License is disabled" << std::endl;
        }
        catch ( LicenseNotFoundException )
        {
            std::cout << "license was not found on the server." << std::endl;
        }
    }

    std::string sInput = "";

    while ( sInput.compare( "e" ) != 0 )
    {
        std::cout << "Press 1 to list product versions, 2 to get product version installation file, 3 to check for newest version, or e to exit." << std::endl;
        std::cout << ">";
        std::getline( std::cin, sInput );

        //After each user input, we'll want to make sure our pointer to the local license is updated.
        try
        {
            license = licenseManager->getCurrentLicense();
        }
        catch ( LocalLicenseException )
        {
            std::cout << "Could not read previous local license. Local license may be corrupt. "
                << "Create a new local license by activating your license." << std::endl;
        }

        //Here we list all the versions for the selected product
        if ( sInput.compare( "1" ) == 0 )
        {
            std::vector<std::string> listVersions = licenseManager->getVersionList( licenseId );
            for ( std::string i : listVersions )
                std::cout << i << std::endl;

        }

        //We allow the user to input the version they wish to receive the installation URL for
        //url could be replaced with required version, md5hash, release date, environment, etc
        else if ( sInput.compare( "2" ) == 0 )
        {
            std::string vInput = "";
            std::cout << "Input the version number you want to switch to: " << std::endl;
            std::getline( std::cin, vInput );
            try 
            {
                InstallationFile::ptr_t ins = licenseManager->getInstallationFile( licenseId, vInput );
                std::cout << "The URL for this installation file is: " << ins->url() << std::endl;
            }
            catch ( ProductVersionException )
            {
                std::cout << "Version not found." << std::endl;
            }
            catch ( ProductNotFoundException )
            {
                std::cout << "Product not found." << std::endl;
            }
            catch ( LicenseNotFoundException )
            {
                std::cout << "License not found." << std::endl;
            }
            catch ( LicenseStateException )
            {
                std::cout << "License disabled." << std::endl;
            }
            catch ( ... )
            {
                std::cout << "Network error with receiving product version installation file." << std::endl;
            }
            
        }
        //Inputting '3' will automatically check if the license is within a maintenance period, and if not it checks for whether
        //the application is being run on the newest version. If not, the program prompts the user to update by giving the url.
        else if ( sInput.compare( "3" ) == 0 )
        {
            //After checking the license to confirm its accuracy, the maintenance period time is stored.
            LicenseCheck( license );
            struct tm time = license->maintenancePeriod();
            time_t t = mktime( &time );
            //If the maintenance period is in the past or was never set (by default maintenance period is NULL)
            if( license->isMaintenancePeriodExpired() || ctime( &t ) == NULL ) 
            {
                //Retrieving the newest version available 
                InstallationFile::ptr_t ins = licenseManager->getInstallationFile( licenseId );
                //If the current application version being run is not equal to the newest version available.
                if ( ( ins->version() ).compare( pConfiguration->getAppVersion() ) != 0 )
                {
                    std::cout << "You are currently on version " << pConfiguration->getAppVersion() << ", which is outdated." << std::endl;
                    std::cout << "The most recent version, " << ins->version() << ", is available now on the " << ins->channel() << " channel." << std::endl;
                    std::cout << "To download this new version or see other product versions, follow this URL: " << ins->url() << std::endl;
                }
                else
                {
                    std::cout << "You are currently running the most recent version of the product." << std::endl;
                }
            }
            else
            {
                printf( "Currently under maintenance!\n" );
                printf( "Maintenance will be finished at %s\n", ctime( &t ) );
            }
        }
        // If 'e' is input we exit the program
        else
            if ( sInput.compare( "e" ) != 0 )
                std::cout << "Unrecognized command." << std::endl;
    }
    return 0;
}

void LicenseCheck( License::ptr_t license )
{
    //First we'll run a online check. This will check your license on the 
    //LicenseSpring servers, and sync up your local license to match your online
    if ( license != nullptr )
    {
        try
        {
            std::cout << "Checking license online..." << std::endl;
            license->check();
            std::cout << "License successfully checked" << std::endl;
        }
        catch ( LicenseStateException )
        {
            std::cout << "Online license is not valid" << std::endl;
            if ( !license->isActive() )
                std::cout << "License is inactive" << std::endl;
            if ( license->isExpired() )
                std::cout << "License is expired" << std::endl;
            if ( !license->isEnabled() )
                std::cout << "License is disabled" << std::endl;
        }

        //We use localCheck() in LicenseSpring/License.h in the include folder.This is
        //useful to check if the license hasn't been copied over from another device, and 
        //that the license is still valid. Note: valid and active are not the same thing. See
        //tutorial [link here] to learn the difference.
        try
        {
            std::cout << "License successfully loaded, performing local check of the license..." << std::endl;
            license->localCheck();
            std::cout << "Local validation successful" << std::endl;
        }
        catch ( LicenseStateException )
        {
            std::cout << "Current license is not valid" << std::endl;
            if ( !license->isActive() )
                std::cout << "License is inactive" << std::endl;
            if ( license->isExpired() )
                std::cout << "License is expired" << std::endl;
            if ( !license->isEnabled() )
                std::cout << "License is disabled" << std::endl;
        }
        catch ( ProductMismatchException )
        {
            std::cout << "License does not belong to configured product." << std::endl;
        }
        catch ( DeviceNotLicensedException )
        {
            std::cout << "License does not belong to current computer." << std::endl;
        }
        catch ( VMIsNotAllowedException )
        {
            std::cout << "Currently running on VM, when VM is not allowed." << std::endl;
        }
        catch ( ClockTamperedException )
        {
            std::cout << "Detected cheating with system clock." << std::endl;
        }
    } 
}