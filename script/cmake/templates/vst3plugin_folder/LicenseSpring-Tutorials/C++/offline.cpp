#include <LicenseSpring/Configuration.h>
#include <LicenseSpring/EncryptString.h>
#include <LicenseSpring/LicenseManager.h>
#include <LicenseSpring/Exceptions.h>
#include <iostream>
#include <thread>

//These headers are only used to convert strings into wstrings
#include <locale> 
#include <codecvt>
#include <string>


using namespace LicenseSpring;

void LocalLicenseCheck( License::ptr_t license );

//Chatbot tutorial for offline licensing. 
//Offline Portal link: https://saas.licensespring.com/offline/
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

    //getCurrentLicense() will return a pointer to the local license stored
    //on the end-user's device if they have one that matches the current 
    //configuration i.e. API key, Shared key, and product code.
    License::ptr_t license = nullptr;
    try
    {
        license = licenseManager->reloadLicense();
    }
    catch ( LocalLicenseException )
    { //Exception if we cannot read the local license or the local license file is corrupt
        std::cout << "Could not read previous local license. Local license may be corrupt. "
            << "Create a new local license by activating your license." << std::endl;
        return 0;
    }

    //Do a quick local check on start up to make sure everything seems right.
    //Note, that we we're only doing a localcheck in our offline tutorial.
    LocalLicenseCheck( license );

    std::string sInput = "";

    while ( sInput.compare( "e" ) != 0 )
    {
        //If license is a nullptr, meaning we don't have a local license, or our license is inactive we'll
        //let the user know and show them how many licenses have been activated, and how many there are total.
        if ( license == nullptr || !license->isActive() )
        {
            std::cout << "Your license is currently inactive, type '1' create offline "
                << "activation request file,'2' to submit offline activation response "
                << "file, or 'e' to exit." << std::endl;

            if ( license != nullptr )
                std::cout << "You currently have " << license->timesActivated()
                << " activated licenses and a maximum of "
                << license->maxActivations() << "." << std::endl;
        }
        else if ( license->isActive() )
        { //Case where license is activated on end-user's device.
            std::cout << "Your license is currently active, type 'c' to check license, "
                << "'d' to deactivate license, '3' to submit offline refresh file, "
                << "or 'e' to exit." << std::endl;
            std::cout << "You currently have " << license->timesActivated() << " activated licenses and a maximum of "
                << license->maxActivations() << "." << std::endl;
        }
        std::cout << ">";
        std::getline( std::cin, sInput );

        //After each user input, we'll want to make sure our pointer to the local license is updated.
        try
        {
            license = licenseManager->reloadLicense();
        }
        catch ( LocalLicenseException )
        {
            std::cout << "Could not read previous local license. Local license may be corrupt. "
                << "Create a new local license by activating your license." << std::endl;
        }

        //Here we check our local license and see if it's valid. 
        if ( sInput.compare( "c" ) == 0 )
        {
            LocalLicenseCheck( license );
        }

        //We will check if the license is currently active. If not we'll
        //delete the local license copy on our device, then create a deactivation request
        //file that will need to be uploaded to the LicenseSpring offline portal to let
        //the platform know that the license on this device is no longer active.
        else if ( sInput.compare( "d" ) == 0 )
        {
            if ( license != nullptr && license->isActive() )
            {
                std::cout << "Creating offline deactivation request file..." << std::endl;

                //Here we'll let the user input their path/name for their deactivation request file
                //and we'll convert it into a wstring which is required for deactivateOffline(path).
                std::string string_path;
                std::cout << "Please input the path you want for you offline deactivation request file "
                    << "or type 'default' to use the default path and name." << std::endl;
                std::cout << ">";
                std::getline( std::cin, string_path );
                
                //We'll also give the user a default option. If path is null, then the default path that
                // deactivateOffline will create the file is:
                //'C:\Users\%USERNAME%\Desktop\ls_deactivation.req'.
                if ( string_path.compare( "default" ) == 0 ) 
                {
                    //default path: C:\Users\%USERNAME%\Desktop\ls_deactivation.req 
                    auto filePath = license->deactivateOffline();
                }
                else 
                { //otherwise we'll convert our input string into a wstring
                    std::wstring path;
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    path = converter.from_bytes( string_path );
                    auto filePath = license->deactivateOffline( path );
                }

                //and we clearLocalStorage to get rid of the local license file on our device.
                
                licenseManager->clearLocalStorage();
                std::cout << "To finish deactivation process please upload deactivation request "
                    << "file to the LicenseSpring portal." << std::endl;
            }
            else
                std::cout << "License is already deactivated." << std::endl;
        }

        //We will check if we have a local license file. If we don't, we'll start creating an 
        //activation request file which will need to be uploaded to the LicenseSpring 
        //offline portal to receive an activation response file. Uploading the request file
        //will activate your license for that device on the LicenseSpring platform (unless
        //an error occured, see 'Reasons Why The Request File May Not Generate a License File 
        //When Uploaded' in our tutorial: [link here]). Note this will not create a local license file
        //on the end-user's device, and so, on their end they will remain inactive until they
        //upload their activation response file.
        else if ( sInput.compare( "1" ) == 0 )
        {
            if ( license == nullptr ) 
            {
                std::cout << "Creating offline activation request file..." << std::endl;

                //Similarly to creating a deactivation request file, we can select a path for the file
                //to be created in, or use a default path (by setting path to null) which is:
                //'C:\Users\%USERNAME%\Desktop\ls_activation.req'
                std::string string_path;
                std::cout << "Please input the path you want for you offline activation request file "
                    << "or type 'default' to use the default path and name." << std::endl;
                std::cout << ">";
                std::getline( std::cin, string_path );
                
                std::wstring filePath;
                if ( string_path.compare( "default" ) == 0 ) 
                {
                    //Here, we'll create the offline activation request file to the default path.
                    //default path: C:\Users\%USERNAME%\Desktop\ls_activation.req 
                    filePath = licenseManager->createOfflineActivationFile( licenseId );
                }
                else 
                {
                    std::wstring path;
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    path = converter.from_bytes( string_path );
                    //Here, we'll create the offline activation request file to our path.
                    filePath = licenseManager->createOfflineActivationFile( licenseId, path );
                }

                std::cout << "File created: " << std::endl;
                std::wcout << filePath << std::endl;
                std::cout << "Please upload that request file to the LicenseSpring portal to get response file." << std::endl;
                return 0;
            } 
            else
                std::cout << "License is already activated." << std::endl;
            
        }
        
        //Here, if our local license isn't created, we can use the activation response file we should've
        //received from the offline portal (given all our information is correct). Once we run 
        //activateLicenseOffline, we'll create a local license file and activate it on our device.
        else if ( sInput.compare( "2" ) == 0 )
        {
            if ( license == nullptr ) 
            {
                //Similar to creating activation/deactivion request file, the default path will be:
                // 'C:\Users\%USERNAME%\Desktop\ls_activation.lic'.
                std::string string_path;
                std::cout << "Please input the path to your offline activation response file "
                    << "or type 'default' to use the default path and name." << std::endl;
                std::cout << ">";
                std::getline( std::cin, string_path );
                
                if  ( string_path.compare( "default" ) == 0 ) 
                {
                    //default path: C:\Users\%USERNAME%\Desktop\ls_activation.lic 
                    try
                    {
                        //This will find the activation response file using the defuault 
                        //path, then create our local license file and point license to it.
                        //If we were unsuccesful, license will be null.
                        license = licenseManager->activateLicenseOffline();
                    }
                    catch ( SignatureMismatchException )
                    {
                        std::cout << "Signature inside activation file is not valid." << std::endl;
                    }
                }
                else 
                {
                    std::wstring path;
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    path = converter.from_bytes( string_path );
                    try
                    {
                        //This will find the activation response file using path, then 
                        //create our local license file and point license to it.
                        //If we were unsuccesful, license will be null.
                        license = licenseManager->activateLicenseOffline( path );
                    }
                    catch ( SignatureMismatchException )
                    {
                        std::cout << "Signature inside activation file is not valid." << std::endl;
                    }
                }
                
                if ( license == nullptr )
                {
                    std::cout << "Activation Response File was not found, make sure you've "
                        << "created the file by entering '1', and that you have set "
                        << "the path correctly." << std::endl;
                }
                else
                {
                    std::cout << "Offline Activation Succesful." << std::endl;
                }
            }
            else 
                std::cout << "License already activated." << std::endl;
            
        }

        //If we change features in our license and we want them to update on offline devices,
        //we can create an update response file as explained here: 
        //https://docs.licensespring.com/docs/offline-licensing.
        else if ( sInput.compare( "3" ) == 0 )
        {
            if ( license == nullptr ) 
            {
                std::cout << "Cannot refresh an inactive license." << std::endl;
            }
            else
            {
                std::string string_path;
                std::cout << "Please input the path to your offline update response file: "
                    << std::endl;
                std::cout << ">";
                std::getline( std::cin, string_path );

                std::wstring path;
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                path = converter.from_bytes( string_path );

                //Here, we update our local license using the offline update response file.
                //If we succeed, it'll return true, otherwise false. 
                try 
                {
                    if ( license->updateOffline( path ) )
                    {
                        std::cout << "License succesfully updated." << std::endl;
                    }
                    else
                    {
                        std::cout << "License could not be updated." << std::endl;
                    }
                }
                catch ( SignatureMismatchException )
                {
                    std::cout << "Device does not match update file's device signature." << std::endl;
                }
            }
        }
        else
            if ( sInput.compare( "e" ) != 0 )
                std::cout << "Unrecognized command." << std::endl;
    }
}

//Performs a localcheck, but with all cases covered, so we don't crash
void LocalLicenseCheck( License::ptr_t license )
{
    //We use localCheck() in LicenseSpring/License.h in the include folder.This is
    //useful to check if the license hasn't been copied over from another device, and 
    //that the license is still valid. Note: valid and active are not the same thing. See
    //tutorial [link here] to learn the difference.
    if ( license != nullptr )
    {
        try
        {
            std::cout << "License successfully loaded, performing local check of the license..." << std::endl;
            license->localCheck();
            std::cout << "Local validation successful." << std::endl;
        }
        catch ( LicenseStateException )
        {
            std::cout << "Current license is not valid." << std::endl;
            if ( !license->isActive() )
                std::cout << "License is inactive." << std::endl;
            if ( license->isExpired() )
                std::cout << "License is expired." << std::endl;
            if ( !license->isEnabled() )
                std::cout << "License is disabled." << std::endl;
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
    else
    {
        std::cout << "No local license found." << std::endl;
    }
}