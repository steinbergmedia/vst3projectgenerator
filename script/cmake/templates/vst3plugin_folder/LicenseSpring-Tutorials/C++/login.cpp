//Remember to include these headers, they may have different paths if you are using this code in a different folder.
#include <LicenseSpring/Configuration.h>
#include <LicenseSpring/EncryptString.h>
#include <LicenseSpring/LicenseManager.h>
#include <LicenseSpring/Exceptions.h>
#include <ctime>
#include <time.h>
#include <iostream>
#include <thread>

using namespace LicenseSpring;

// 
//License Checking function at bottom of code. Shows how to do an online check and sync, as well as a local check.
void LicenseCheck( License::ptr_t license );

//Our console ChatBot login program using LicenseSpring activation/deactivations/checking
int main()
{
    std::string appName = "NAME"; //input name of application
    std::string appVersion = "VERSION"; //input version of application

    //Collecting network info
    ExtendedOptions options;
    options.collectNetworkInfo( true );

    std::shared_ptr<Configuration> pConfiguration = Configuration::Create(
        EncryptStr( "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXX" ), // your LicenseSpring API key (UUID)
        EncryptStr( "XXXXXXXX-XXXXX-XXXXXXXXXXXX_XXXXXX_XXXXXX" ), // your LicenseSpring Shared key
        EncryptStr( "XXXXXXXXX" ), // product code that you specified in LicenseSpring for your application
        appName, appVersion, options );

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
    
    while ( 1 )
    {
        try
        {
            //We'll need to reload our license from our local storage on each
            //loop to make sure we don't use the local license already loaded
            //into memory if we deactivate and removed our local license
            //from storage.
            license = licenseManager->reloadLicense();
        }
        catch ( LocalLicenseException )
        { //Exception if we cannot read the local license or the local license file is corrupt
            std::cout << "Could not read previous local license. Local license may be corrupt. "
                << "Create a new local license by activating your license." << std::endl;
            return 0;
        }

        //If there is no local license on this device, i.e. license == nulptr, 
        //that user is logged out, and will need to input their credentials to login.
        if ( license == nullptr ) 
        {
            std::cout << "Please enter your log in information." << std::endl;

            std::cout << "Username/Email: ";
            std::string email = "";
            std::getline( std::cin, email );
            
            //If email is empty, we'll just rerun the loop and ask them to input it again.
            if ( email == "" )
                continue;

            std::cout << "Password: ";
            std::string password = "";
            std::getline( std::cin, password );

            //We'll create a licenseID and if the credentials are valid, we'll 
            //activate their license and log them in.
            auto licenseId = LicenseID::fromUser( email, password );
            try
            {
                license = licenseManager->activateLicense(licenseId);
                LicenseUser::ptr_t user = license->licenseUser();
                if (user->isInitialPassword())
                {
                    std::cout << "This account still uses an initial password, type 'y' to set your password." << std::endl;
                    std::string inp = "";
                    std::getline(std::cin, inp);
                    if (inp.compare("y") == 0)
                    {
                        std::cout << "Changing passwords:" << std::endl;
                        std::cout << "Please enter initial password: " << std::endl;
                        std::string old_pass = "";
                        std::getline(std::cin, old_pass);
                        std::cout << "Please enter new password: " << std::endl;
                        std::string new_pass = "";
                        std::getline(std::cin, new_pass);
                        std::cout << "Please retype new password: " << std::endl;
                        std::string re_new_pass = "";
                        std::getline(std::cin, re_new_pass);
                        if (new_pass != re_new_pass)
                        {
                            std::cout << "Passwords do not match." << std::endl;
                        }
                        else
                        {
                            //We'll check if their old password is correct, and change their password if it is.
                            if (license->changePassword(old_pass, new_pass))
                            {
                                std::cout << "Password change accepted. New password: " << new_pass << std::endl;
                            }
                            else
                            {
                                std::cout << "Password change failed." << std::endl;
                            }
                        }
                    }
                }
            }
            catch ( ProductNotFoundException )
            {
                std::cout << "Could not find product on server. "
                    << "Please check to make sure you input the correct API key, shared key, and product code."
                    << std::endl;
            }
            catch ( InvalidCredentialException )
            {
                std::cout << "Incorrect email and/or password." << std::endl;
            }
            catch ( LicenseNotFoundException ) 
            {
                std::cout << "License not found on server" << std::endl;
            }
            catch ( LicenseStateException ) 
            {
                std::cout << "Account/license is expired or disabled." << std::endl;
            }
            catch ( LicenseNoAvailableActivationsException ) 
            {
                std::cout << "Account/license has reached max number of users/activations currently." << std::endl;
            } 
            catch ( ... ) {
                std::cout << "Possible server issue, please try again." << std::endl;
            }

            //If our activation was succesful, license should be pointing to a local license copy.
            if ( license != nullptr ) 
            {
                std::cout << "Successfully logged in on account: " << email << std::endl;
            }
        }
        else //Case where we have a local license file, and therefore we are logged in.
        { 
            //Collect the date that our license was last checked. Note, an activation
            //also includes a check.
            tm last_checked = license->lastCheckDate();
            time_t timer;
            double seconds;

            time( &timer ); 
            
            //The amount of time since our last check in seconds
            seconds = difftime( timer, mktime( &last_checked ) );

            //Note, if you want a longer period, such as days, you can use
            //int days = license->daysPassedSinceLastCheck()
            if ( seconds > 60 ) 
            {
                std::cout << "You have been inactive for longer than 60 seconds, "
                          << "you have been automatically logged out." << std::endl;
                license->deactivate( true ); //We'll deactivate/log off user if they have been idle 
                continue;
            }

            LicenseUser::ptr_t user = license->licenseUser();
            std::cout << "You are currently logged in as " << user->firstName() 
                      << " " << user->lastName() << " (" << user->email() << ")" << std::endl;
            std::cout << "Enter 'e' to exit, enter 'l' to log out, " 
                      << "enter 'p' to change password, and 'c' to check your license and "
                      << "reset your login period" << std::endl;
            std::string sInput = "";
            std::getline( std::cin, sInput );

            //We'll check again to see if the user timed out.
            last_checked = license->lastCheckDate();

            time( &timer );
            seconds = difftime(timer, mktime(&last_checked));

            if (seconds > 60)
            {
                std::cout << "You have been inactive for longer than 60 seconds, "
                    << "you have been automatically logged out." << std::endl;
                license->deactivate(true);
                continue;
            }

            if ( sInput.compare( "e" ) == 0 ) 
            {
                return 0;
            }
            else if ( sInput.compare( "l" ) == 0 ) 
            {
                //We can log out a user by deactiving their license and deleting their local
                //license file by setting the parameter in deactivate to be true.
                std::cout << "Logging out" << std::endl;
                license->deactivate( true );
                std::cout << "Logged out." << std::endl;
            }
            else if ( sInput.compare( "p" ) == 0 ) 
            {
                std::cout << "Changing passwords:" << std::endl;
                std::cout << "Please enter old password: " << std::endl;
                std::string old_pass = "";
                std::getline( std::cin, old_pass );
                std::cout << "Please enter new password: " << std::endl;
                std::string new_pass = "";
                std::getline( std::cin, new_pass );
                std::cout << "Please retype new password: " << std::endl;
                std::string re_new_pass = "";
                std::getline( std::cin, re_new_pass );
                if ( new_pass != re_new_pass )
                {
                    std::cout << "Passwords do not match." << std::endl;
                }
                else 
                {
                    try 
                    {
                        //We'll check if their old password is correct, and change their password if it is.
                        if ( license->changePassword( old_pass, new_pass ) )
                        {
                            std::cout << "Password change accepted. New password: " << new_pass << std::endl;
                        }
                        else
                        {
                            std::cout << "Password change failed." << std::endl;
                        }
                    }
                    catch ( InvalidCredentialException )
                    {
                        std::cout << "Incorrect old password." << std::endl;
                    }

                }
            }
            else if ( sInput.compare( "c" ) == 0 ) 
            {
                //Perform an online check and offline local check
                LicenseCheck( license );
            }
            else 
            {
                std::cout << "Unrecognized command." << std::endl;
            }
        }
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
    else 
    {
        std::cout << "No local license found";
    }
}
