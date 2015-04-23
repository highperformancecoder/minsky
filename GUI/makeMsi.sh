# Build an MSI installer from a compiled Minsky
# Must be run from the toplevel directory of Minsky
PATH=$PATH:/c/cygwin/bin
productId=`uuidgen`
upgradeId=01a8458a-5fb5-49e6-a459-531a16e2ea01
componentId=`uuidgen`
version=`cut -f3 -d' ' minskyVersion.h|head -1|tr -d '"'|tr -d "D"`
if [ $version=='"unknown"' ]; then
    version=0.0.0.0
fi
minskyWxs=`pwd`/minsky.wxs

cat >$minskyWxs <<EOF
<?xml version='1.0' encoding='windows-1252'?>
<Wix xmlns='http://schemas.microsoft.com/wix/2006/wi'>
<Product Name='Minsky' Id='$productId' UpgradeCode='$upgradeId'
    Language='1033' Codepage='1252' Version='$version' Manufacturer='High Performance Coders'>
    <Package Id='*' Keywords='Installer' Description="Minsky's Installer"
      Comments='Minsky is copyright Steve Keen, and licensed under GPL3' Manufacturer='High Performance Coders'
      InstallerVersion='100' Languages='1033' Compressed='no' SummaryCodepage='1252' />
    <Upgrade Id='$upgradeId'>
      <UpgradeVersion OnlyDetect='no' Property='PREVIOUSFOUND'
         Minimum='0.0.0' IncludeMinimum='yes'
         Maximum='2.0.0' IncludeMaximum='no' />
    </Upgrade>
    <InstallExecuteSequence>
      <RemoveExistingProducts Before='InstallInitialize'/>
    </InstallExecuteSequence>
    <Media Id='1' Cabinet='minsky.cab' EmbedCab='yes'/>
    <Directory Id='TARGETDIR' Name='SourceDir'>
      <Directory Id='ProgramFilesFolder'>
        <Directory Id='Minsky' Name='Minsky'>
          <Directory Id='INSTALLDIR'>

            <Component Id='MinskyFiles' Guid='$componentId'>
              <File Id='MinskyEXE' Name='minsky.exe' Source='GUI/minsky.exe' KeyPath='yes'>
                <Shortcut Id="startmenuMinsky" Directory="ProgramMenuDir" Name="Minsky" WorkingDirectory='INSTALLDIR' Icon="minsky.exe" IconIndex="0" Advertise="yes">
                 </Shortcut>
                <Shortcut Id="desktopMinsky" Directory="DesktopFolder" Name="Minsky" WorkingDirectory='INSTALLDIR' Icon="minsky.exe" IconIndex="0" Advertise="yes" />
              </File>
              <ProgId Id='MinskyData' Description='Minsky Project File' Icon='MinskyEXE'>
                <Extension Id='mky' ContentType='application/minsky'>
                  <Verb Id='open' Command='Open' TargetFile='MinskyEXE' Argument='"%1"'/>
                 </Extension>
               </ProgId>
EOF
pushd GUI
id=0
fid=0
# add in plain files
for i in *.tcl accountingRules; do
	if [ ! -d $i -a $i != "minsky.exe" ]; then
	    let fid++
	    cat >>$minskyWxs <<EOF
	<File Id='fid$fid' Source='GUI/$i' Name='$i' KeyPath='no' />
EOF
        fi
done
echo "    </Component>">> $minskyWxs
builddir ()
{
    dir=${1##*/}
    pushd $dir
    let d++
    echo "<Directory Id='id$d' Name='$dir'>" >>$minskyWxs
    # first process files into a single component
    j=0
    for i in *; do 
	if [ -f $i ]; then
	    let fid++
            if [ $j -eq 0 ]; then
                let id++
                echo "<Component Id='id$id' Guid='`uuidgen`'>">>$minskyWxs
 	        echo "<File Id='fid$fid' Source='GUI/$1/$i' Name='$i' KeyPath='yes' />">>$minskyWxs
           else
	        echo "<File Id='fid$fid' Source='GUI/$1/$i' Name='$i' KeyPath='no' />">>$minskyWxs
            fi
            let j++
	fi
    done
    if [ $j -gt 0 ]; then
        echo "</Component>">>$minskyWxs
    fi

    # then process subdirectories 
    for i in *; do 
	if [ -d $i ]; then
	    builddir $1/$i
        fi
    done
    echo "</Directory>">>$minskyWxs
    popd
}

builddir library
builddir icons
popd

cat >>$minskyWxs <<EOF
          </Directory>
        </Directory>
        
      </Directory>

      <Directory Id="ProgramMenuFolder" Name="Programs">
        <Directory Id="ProgramMenuDir" Name="Minsky">
          <Component Id="ProgramMenuDir" Guid="40e1115c-9edf-4ae9-b4d3-6508f1921f51">
            <RemoveFolder Id='ProgramMenuDir' On='uninstall' />
            <RegistryValue Root='HKCU' Key='Software\[Manufacturer]\[ProductName]' Type='string' Value='' KeyPath='yes' />
          </Component>
        </Directory>
      </Directory>

      <Directory Id="DesktopFolder" Name="Desktop" />
    </Directory>

    <Feature Id='Complete' Level='1'>
      <ComponentRef Id='MinskyFiles' />
      <ComponentRef Id='ProgramMenuDir' />
EOF
i=0
while [ $i -lt $id ]; do
    let i++
    echo "<ComponentRef Id='id$i' />">>$minskyWxs 
done
cat >>$minskyWxs <<EOF
    </Feature>

    <Icon Id="minsky.exe" SourceFile="GUI/minsky.exe" />

   <UI>
      <UIRef Id='WixUI_InstallDir'/>
    </UI>
    <Property Id="WIXUI_INSTALLDIR" Value="INSTALLDIR"/>
  </Product>
</Wix>
EOF


candle minsky.wxs
echo "light minsky.wixobj"
light -ext WixUIExtension -dWixUILicenseRtf=license.rtf minsky.wixobj
mv minsky.msi Minsky.$version-win-dist.msi
