#include <LicenseSpring/Configuration.h>
#include <LicenseSpring/EncryptString.h>
#include <LicenseSpring/LicenseManager.h>
#include <LicenseSpring/Exceptions.h>
#include <iostream>
#include <thread>

using namespace LicenseSpring;

//Sample code tutorial that demonstrates obtaining Custom Field values and creating/sending
//Device Variables to the LicenseSpring platform.
int main()
{
    std::string appName = "NAME"; //input name of application
    std::string appVersion = "VERSION"; //input version of application

    //Collecting network info
    ExtendedOptions options;
    options.collectNetworkInfo( true );

    std::shared_ptr<Configuration> pConfiguration = Configuration::Create(
        EncryptStr("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"), // your LicenseSpring API key (UUID)
        EncryptStr("XXXXXXXXX-XXXXX-XXXXXXXXXXXXX_XXXXXX_XXXXXX"), // your LicenseSpring Shared key
        EncryptStr("XXXXXX"), // product code that you specified in LicenseSpring for your application
        appName, appVersion, options);

    //Key-based implementation
    auto licenseId = LicenseID::fromKey("XXXX-XXXX-XXXX-XXXX"); //input license key

    //For user-based implementation comment out above line, and use bottom 3 lines
 //   const std::string userId = "example@email.com"; //input user email
 //   const std::string userPassword = "password"; //input user password
 //   auto licenseId = LicenseID::fromUser( userId, userPassword );

    auto licenseManager = LicenseManager::create( pConfiguration );

    //reloadLicense() will return a pointer to the local license stored
    //on the end-user's device if they have one that matches the current 
    //configuration i.e. API key, Shared key, and product code.
    License::ptr_t license = nullptr;

    try
    {
        license = licenseManager->reloadLicense();
        if ( license != nullptr )
        {
            license->localCheck(); //always good to do a local check whenever you run your program 
        }
    }
    catch ( LocalLicenseException )
    { //Exception if we cannot read the local license or the local license file is corrupt
        std::cout << "Could not read previous local license. Local license may be corrupt. "
            << "Create a new local license by activating your license." << std::endl;
        return 0;
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

    //Here we'll make sure our license is activated before we start testing with 
    //custom fields and device variables.
    try 
    {
        license = licenseManager->activateLicense( licenseId );
        
    }
    catch ( ProductNotFoundException )
    {
        std::cout << "Product not found on server, please check your product configuration." << std::endl;
        return 0;
    }
    catch ( LicenseNotFoundException )
    {
        std::cout << "License could not be found on server." << std::endl;
        return 0;
    }
    catch ( LicenseStateException )
    {
        std::cout << "License is currently expired or disabled." << std::endl;
        return 0;
    }
    catch ( LicenseNoAvailableActivationsException )
    {
        std::cout << "No available activations remaining on license." << std::endl;
        return 0;
    }
    catch ( CannotBeActivatedNowException )
    {
        std::cout << "Current date is not at start date of license." << std::endl;
        return 0;
    }
    catch ( SignatureMismatchException )
    {
        std::cout << "Signature on LicenseSpring server is invalid." << std::endl;
        return 0;
    }
    catch ( ... )
    {
        std::cout << "Possible connection issue." << std::endl;
        return 0;
    }

    
    //Once our local license is synced up/updated with the online license, we should have the most up-to-date
    //custom fields. There are two ways to update our custom fields to be in sync with our online product/license
    //1. Activating a license will update all our custom fields on our local license.
    //2. Running an online check (as long as it doesn't throw an exception), will also sync up our custom fields.
    //on our local license.
    if( license != nullptr ) 
    {
        license->check();
    }
    //We can then extract our custom fields as a vector of CustomField objects as so:
    std::vector<CustomField> custom_vec = license->customFields();

    //Note: it is possible for our custom fields to be empty if we haven't added anything so be aware.
    for ( CustomField custom_field : custom_vec )
    {
        //Here we display our key:value pairs.
        std::cout << "Key: " << custom_field.fieldName() << " |Value: " << custom_field.fieldValue() << std::endl;
        
        //You can also update the custom field key:value pairs as well, however, this 
        //will only affect the custom fields locally on your device, and will not in any way change
        //the custom field key:value pairs set up on the LicenseSpring platform. The only way to change
        //the custom field values on the LicenseSpring platoform, is manually, and not with code.
        custom_field.setFieldName( "New " + custom_field.fieldName() );
        custom_field.setFieldValue( "New " + custom_field.fieldValue() );
        std::cout << "New Key: " << custom_field.fieldName() << "|New Value: " << custom_field.fieldValue() << std::endl;
    }


    
    //Since device variables are the opposite of custom fields, we create them on the user end then sync with the backend
    std::cout << "Enter 'y' to create a device variable, any other input to skip." << std::endl;
    std::string response = "";
    std::getline( std::cin, response );

    //Through taking in the name and value of the device variable we are able to pass them into addDeviceVariable() to create the device
    //variable locally. The platform still does not contain device variables after addDeviceVariable()

    //To update device variables, use the same varName as the variable you want to update, and use your
    //updated value for varValue.
    if ( response.compare( "y" ) == 0 ) 
    {

        std::cout << "Name of Variable: ";
        std::string varName = "";
        std::getline( std::cin, varName );

        std::cout << "Value of Variable: ";
        std::string varValue = "";
        std::getline( std::cin, varValue );
        license->addDeviceVariable( varName, varValue );
        //Note, you can also create a DeviceVariable object and pass that in as a parameter instead.
        //Furthermore, there is an optional third parameter, that by default is set true. When true,
        //it'll save the DeviceVariable on your local license. When false, it will not add the Device
        //Variable to your local license.
    }

    //sendDeviceVariables() sends the device variables that have been created and stored locally to the backend, which syncs up
    //both ends to have matching device variables. Creating a vector of device variables and setting it to equal license->getDeviceVariables()
    //retrieves all of the device variables currently stored locally. The true that we pass into getDeviceVariables() makes the function check
    //the backend, which contains all of the device variables since we just sent local changes to the backend right before getting them.

    std::vector<DeviceVariable> device_vec;
    try
    {
        license->sendDeviceVariables();
        device_vec = license->getDeviceVariables( true );
    }
    catch ( ... )
    {
        std::cout << "Most likely a network connection issue, please check your connection." << std::endl;
    }
    
    //Looping through each device variable, we are able to print the names and values of each device variable using name() and value()
    //respectively.

    for ( DeviceVariable device_variable : device_vec )
    {
        std::cout << "Device Variable Name: " << device_variable.name() << " |Value: " << device_variable.value() << std::endl;
    }

    return 0;
}
