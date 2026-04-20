#include <LicenseSpring/Configuration.h>
#include <LicenseSpring/EncryptString.h>
#include <LicenseSpring/LicenseManager.h>
#include <LicenseSpring/Exceptions.h>
#include <iostream>
#include <thread>

using namespace LicenseSpring;

//Sample code for a consumption based license. This code will demonstrate how, consumptions can be implemented
//in one's code, including, checking the amount of consumptions at any moment, checking the total amount, 
//checking if we are in overages, and syncing it with the back end.
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


    std::shared_ptr<LicenseManager> licenseManager = LicenseManager::create( pConfiguration );

    //Key-based implementation
    auto licenseId = LicenseID::fromKey( "XXXX-XXXX-XXXX-XXXX" ); //input license key

    //For user-based implementation comment out above line, and use bottom 3 lines
 //   const std::string userId = "example@email.com"; //input user email
 //   const std::string userPassword = "password"; //input user password
 //   auto licenseId = LicenseID::fromUser( userId, userPassword );

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

    //If we don't have a local license yet, we'll try activating it first. Otherwise we'll do an online
    //check to sync up our license with the backend. Note, if you recently reset your license, then you may get
    //an exception here. You can fix this by deleting your local license folder located at 
    //C:\Users\%USERPROFILE%\AppData\Local\LicenseSpring\'product code'.
    try
    {
        if ( license == nullptr )
            license = licenseManager->activateLicense( licenseId );
        else
            license->check();

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

    std::cout << "Type 'e' to exit, type 'y' to increase your consumption." << std::endl;
    std::string sInput = "";
    
    while ( sInput.compare( "e" ) != 0 )
    {
        try
        {
            //This is what we'll use to sync our consumption up with the backend, it will check the backend, as well 
            //as our local licenses consumption count, and makes updates the backend and our local file with the 
            //correct consumption count. Thus, it'll work for more than one device.
            license->syncConsumption( -1 );
            std::cout << "You have used a total of " << license->totalConsumption() << " consumptions so far." << std::endl;
            if ( license->totalConsumption() > license->maxConsumption() )
            {
                //This is the case where the user is in the max-overages. You can do something special in this case
                //, or just notify the user they are in the max-overages.
                std::cout << "You are currently using max overages." << std::endl;
                std::cout << "You have " << license->maxConsumption() + license->maxOverages() - license->totalConsumption() <<
                    " consumptions left." << std::endl;
            }
            else
            {
                //If the user is not in max overages, you can have your normal code.
                std::cout << "You have " << license->maxConsumption() - license->totalConsumption() <<
                    " consumptions left before you are in the overage territory." << std::endl;
            }
        }
        catch ( NotEnoughConsumptionException ) //This exception is pretty useful to find out when you are out of consumptions
        {
            std::cout << "Please reset for more consumptions." << std::endl;
        }
        catch ( LicenseSpringException ex ) 
        {
            std::cout << ex.what() << std::endl;
            return 0;
        }
        std::cout << ">";
        std::getline( std::cin, sInput );

        if ( sInput.compare( "y" ) == 0 )
        {
            try
            {
                //The first parameter will increment or decrement (if negative) our consumption count by that amount.
                //The second parameter, when true, will update our local license file with the new consumption value.
                license->updateConsumption( 1, true );
                std::cout << "You've just used one consumption." << std::endl;
            }
            catch ( NotEnoughConsumptionException )
            {
                std::cout << "You are out of consumptions." << std::endl;
            }
            catch ( LicenseSpringException ex ) 
            {
                std::cout << ex.what() << std::endl;
                return 0;
            }
        }
    }
    return 0;
}
