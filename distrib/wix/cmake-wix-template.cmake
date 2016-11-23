<?xml version='1.0' encoding='windows-1252'?>
<?define ProductVersion = "@nany_version@"?>
<?define ProductUpgradeCode = "7AE565D5-6CF5-45DB-80FD-92891DB1F212"?>
<?define ConfigMode = "@wix_config_mode@"?>
<?define Arch = "@wix_arch@"?>
<Wix xmlns='http://schemas.microsoft.com/wix/2006/wi'>
	<Product Name='Nany Compiler $(var.ProductVersion) $(var.Arch)' Id='DD3F6C0C-38E4-439C-89C2-11AC6B0A33C6' UpgradeCode='$(var.ProductUpgradeCode)'
		Language='1033' Codepage='1252' Version='$(var.ProductVersion)' Manufacturer='Nany Team'>

		<Package Id='*' Keywords='Installer' Description="Nany Compiler $(var.ProductVersion) $(var.Arch) Installer"
			Comments='@nany_website_url@' Manufacturer='Nany Team'
			InstallerVersion='100' Languages='1033' Compressed='yes' SummaryCodepage='1252' />
		<Media Id='1' Cabinet='nanyc.cab' EmbedCab='yes' CompressionLevel="high" />

		<Upgrade Id="$(var.ProductUpgradeCode)">
			<UpgradeVersion Minimum="$(var.ProductVersion)" OnlyDetect="yes" Property="NEWERVERSIONDETECTED"/>
			<UpgradeVersion Minimum="0.0.0" Maximum="$(var.ProductVersion)" IncludeMinimum="yes" IncludeMaximum="no" Property="OLDERVERSIONBEINGUPGRADED"/>	  
		</Upgrade>
		<Condition Message="A newer version of this software is already installed.">NOT NEWERVERSIONDETECTED</Condition>


		<Directory Id='TARGETDIR' Name='SourceDir'>
			<Directory Id='ProgramFilesFolder' Name='PFiles'>
				<Directory Id='APPLICATIONFOLDER' Name='Nany'>
					<Directory Id='INSTALLDIR' Name='$(var.ProductVersion)'>

						<Component Id='MainExecutable' Guid='752A018E-ED34-45CB-911B-DC7E6C174E9E'>
						<File Id='nanycEXE' Name='nanyc.exe' DiskId='1' Source='..\bootstrap\$(var.ConfigMode)\nanyc.exe' KeyPath='yes'>
								<Shortcut Id="startmenuNanyEXE" Directory="ProgramMenuDir" Name="Nany $(var.ProductVersion)" WorkingDirectory='INSTALLDIR' Advertise="yes" />
							</File>
							<File Id='libnanyc' Name='libnanyc.dll' DiskId='1' Source='..\bootstrap\$(var.ConfigMode)\libnanyc.dll' />
						</Component>
						<Directory Id='Examples' Name='Examples'>
							<Component Id='Examples' Guid='7991DA11-8AFF-48A1-A2F8-95B7A8CA31CD'>
								<File Id='exp_00' Name='00-hello-world.ny' DiskId='1' Source='..\examples\00-hello-world.ny' />
								<File Id='exp_01' Name='01-fibonacci.ny' DiskId='1' Source='..\examples\01-fibonacci.ny' />
								<File Id='exp_82' Name='82-properties.ny' DiskId='1' Source='..\examples\82-properties.ny' />
							</Component>
						</Directory>
					</Directory>

				</Directory>
			</Directory>

			<Directory Id="ProgramMenuFolder" Name="Programs">
				<Directory Id="ProgramMenuDir" Name="Nany $(var.ProductVersion)">
					<Component Id="ProgramMenuDir" Guid="C511B65F-B647-4CDE-AE48-0A989EE51F10">
						<RemoveFolder Id='ProgramMenuDir' On='uninstall' />
						<RegistryValue Root='HKCU' Key='Software\[Manufacturer]\[ProductName]' Type='string' Value='' KeyPath='yes' />
					</Component>
				</Directory>
			</Directory>

			<Directory Id="DesktopFolder" Name="Desktop" />
		</Directory>

		<InstallExecuteSequence>
			<RemoveExistingProducts After="InstallValidate"/>
		</InstallExecuteSequence>

		<Feature Id='nanyc' Title='Nany Compiler $(var.ProductVersion) $(var.Arch)' Level='1'>
			<ComponentRef Id='MainExecutable' />
			<ComponentRef Id='ProgramMenuDir' />
		</Feature>
		<Feature Id='Examples' Title='Examples' Level='100'>
			<ComponentRef Id='Examples' />
		</Feature>

		<!--<UIRef Id="WixUI_Mondo" />
		<UIRef Id="WixUI_ErrorProgressText" />-->
		<WixVariable Id="WixUILicenseRtf" Value="../assets/license/license.rtf" />
		<WixVariable Id="WixUIBannerBmp"  Value="../distrib/wix/blank.bmp" />
		<WixVariable Id="WixUIDialogBmp"  Value="../distrib/wix/dialog.bmp" />
		<Property Id="WixAppFolder" Value="WixPerMachineFolder" />
		<Property Id="ApplicationFolderName" Value="Nany" />

		<UI Id='Mondo'>
			<UIRef Id="WixUI_Mondo" />
			<UIRef Id="WixUI_ErrorProgressText" />
			<Publish Dialog="WelcomeDlg" Control="Next" Event="NewDialog" Value="SetupTypeDlg" Order="3">1</Publish>
			<Publish Dialog="SetupTypeDlg" Control="Back" Event="NewDialog" Value="WelcomeDlg" Order="3">1</Publish>		
		</UI>
	</Product>
</Wix>
