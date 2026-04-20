#include <LicenseSpring/Configuration.h>
#include <LicenseSpring/EncryptString.h>
#include <LicenseSpring/LicenseManager.h>
#include <LicenseSpring/Exceptions.h>
#include <iostream>
#include <thread>

using namespace LicenseSpring;

//Code sample for creating a trial license. 
//Note, for user-based trials, you need to make sure the user account is still using thier initial password, and not a changed password.
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

    //For key-based trial license, you can leave user as nullptr.
    //For user-based trial license, you must replace nullptr with Customer("example@email.com"), 
    //where the parameter is your user-based email. User-based trials need to have an email associated with them.
    //const std::string email = "";
    Customer::ptr_t user = nullptr;

    
    //If you leave as std::string() - an empty string, you will use the default license policy
    //on your product. You can change that by putting your license policy code as a string.
    const std::string license_policy_code = std::string();

    LicenseID licenseId;
    
    //This is where we will request a trial license. We will also check to see if our current product
    //and license policy allows trials.
    try
    {
        licenseId = licenseManager->getTrialLicense( user, license_policy_code );
        //licenseId = licenseManager->getTrialLicense( email );
    }
    catch ( TrialNotAllowedException )
    {
        std::cout << "Trial is not allowed on current product + license_policy combination" << std::endl;
        return 0;
    }
    catch ( ProductNotFoundException )
    {
        std::cout << "Product could not be found on server. Please make sure your API key, Shared key, and "
            << "Product Code are all correct." << std::endl;
        return 0;
    }
    catch ( MissingEmailException )
    {
        std::cout << "User based products require an email, even for trial licenses." << std::endl;
    }
    catch ( ... )
    {
        std::cout << "Possible network issue." << std::endl;
        return 0;
    }

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
            std::cout << "Local check complete." << std::endl;
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

    //We add this condition, because if we previously had an active, non-trial license, 
    //running activateLicense with the current set-up will overwrite your non-trial license
    //with a trial license. activateLicense does not stop a non-trial license from being overwritten
    //by a trial license, so it is ono the developer to make sure that, that does not occur.
    //Note, if our previous license was a trial, we are okay with overwriting it. If the allow 
    //multiple licenses on trial is on, we will actually create a brand new license on the 
    //LicenseSpring platform. If it is off, we'll continue to use our previous trial license.
    //If that previous license is expired, we will not be able to activate our license again.
    //See [link to trial tutorial here] for more information.
    if ( license == nullptr || license->isTrial() ) 
    {
        license = licenseManager->activateLicense( licenseId );
        std::cout << "Creating/updating trial license." << std::endl;
    }

    //You can check for if a license is a trial, and based off that, tailor the response.
    if ( license->isTrial() )
    {
        //Here is where you would put your trial code for a user.
        std::cout << "Currently using a trial license." << std::endl;
        std::cout << "Showing limited options." << std::endl;
    }
    else
    {
        //Here is where you would put your non-trial code for a user.
        std::cout << "Currently using a non-trial license." << std::endl;
        std::cout << "Showing full account options." << std::endl;
    }

    return 0;
}
