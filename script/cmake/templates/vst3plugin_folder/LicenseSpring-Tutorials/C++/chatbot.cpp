//Remember to include these headers, they may have different paths if you are using this code in a different folder.
#include <LicenseSpring/Configuration.h>
#include <LicenseSpring/EncryptString.h>
#include <LicenseSpring/LicenseManager.h>
#include <LicenseSpring/Exceptions.h>
#include <iostream>
#include <thread>

using namespace LicenseSpring;

//License Checking function at bottom of code. Shows how to do an online check and sync, as well as a local check.
void LicenseCheck( License::ptr_t license );

//Our console ChatBot program that allows the user to activate, deactivate, and check their LicenseSpring license.
int main() 
{
    std::string appName = "NAME"; //input name of application
    std::string appVersion = "VERSION"; //input version of application

    //Collecting network info
    ExtendedOptions options;
    options.collectNetworkInfo( true );

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

    auto licenseManager = LicenseManager::create( pConfiguration );
  
    //Collects license product info from LicenseSpring servers. Throws
    //an exception if the product could not be found on the LicenseSpring 
    //servers. The rest of the exceptions are all network based exceptions. 
    //See LicenseSpring/BaseManager.h. in the include folder 
    //for more information.
    ProductDetails productInfo;

    try 
    {
        productInfo = licenseManager->getProductDetails();
    }
    catch ( ProductNotFoundException ) 
    {
        std::cout << "Could not find product on server. " 
                  << "Please check to make sure you input the correct API key, shared key, and product code."
                  << std::endl;
        return 0;
    } 
    catch ( ConfigurationException )
    {
        std::cout << "Could not find product on server. "
                  << "Please check to make sure you input the correct API key, shared key, and product code."
                  << std::endl;
        return 0;
    }
    std::cout << "Welcome to the C++ LicenseSpring Introduction Chatbot." << std::endl;

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

    std::string sInput = "";

    while ( sInput.compare( "e" ) != 0 ) 
    {

        //If license is a nullptr, meaning we don't have a local license, or our license is inactive we'll
        //let the user know and show them how many licenses have been activated, and how many there are total.
        if ( license == nullptr || !license->isActive() ) 
        {
            std::cout << "Your license is currently inactive, type 'a' to activate license, or 'e' to exit." << std::endl;
            if ( license != nullptr )
                std::cout << "You currently have " << license->timesActivated() 
                          << " activated licenses and a maximum of " 
                          << license->maxActivations() << "." << std::endl;
        }
        else if ( license->isActive() ) 
        { //Case where license is activated on end-user's device.
            std::cout << "Your license is currently active, type 'c' to check license, "
                      << "'d' to deactivate license, or 'e' to exit." << std::endl;
            std::cout << "You currently have " << license->timesActivated() << " activated licenses and a maximum of " 
                      << license->maxActivations() << "." << std::endl;
        }

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
        
        //Here we check our local license and see if it's valid. 
        if ( sInput.compare("c") == 0 ) 
        {
            LicenseCheck(license);
        }

        //We will check if the license is currently active and deactivate it
        //if it is. See tutorial [link here] to do offline deactivation.
        else if ( sInput.compare( "d" ) == 0 ) 
        {
            if ( license != nullptr && license->isActive() ) 
            {
                //Deactivates license on LicenseSpring servers. If the parameter is 
                //true, it also deletes all LicenseSpring created files on device.
                if ( license->deactivate( true ) )
                    std::cout << "License deactivated successfully." << std::endl;
            }
            else
                std::cout << "License is already deactivated." << std::endl;
        }

        //We will check if the license is currently inactive and activate it
        //if it is. See tutorial [link here] to do online activation
        else if ( sInput.compare( "a" ) == 0 ) 
        {
            if ( license == nullptr || !license->isActive() ) 
            {
                try {
                    //Activates license on LicenseSpring servers and creates/updates pointer
                    //to local license file. Throws an exception if we are over the max number of activations
                    license = licenseManager->activateLicense(licenseId);
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
                catch ( LicenseNotFoundException) 
                {
                    std::cout << "license was not found on the server." << std::endl;
                }       
            }
            else   
                std::cout << "Program is already activated." << std::endl;
        }
        else
            if( sInput.compare( "e" ) != 0 ) 
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
