#  @copyright Steve Keen 2012
#  @author Russell Standish
#  This file is part of Minsky.
#
#  Minsky is free software: you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Minsky is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Minsky.  If not, see <http://www.gnu.org/licenses/>.
#


set fname ""
set workDir [pwd]
encoding system utf-8

# On mac-build versions, fontconfig needs to find its config file,
# which is packaged up in the Minsky.app directory

if {$tcl_platform(os)=="Darwin" && [file exists $minskyHome/../Resources/fontconfig/fonts.conf]} {
        set env(FONTCONFIG_FILE) $minskyHome/../Resources/fontconfig/fonts.conf
}

if {[minsky.ravelVersion]=="unavailable"} {
    set progName Minsky
} else {
    set progName Ravel
}

# default canvas size. Overridden by previously resized window size
# saved in .minskyrc
set canvasWidth 600
set canvasHeight 800
set backgroundColour lightGray
set preferences(nRecentFiles) 10
set preferences(panopticon) 0
set preferences(focusFollowsMouse) 0
set preferences(multipleEquities) 0
set recentFiles {}

# select Arial Unicode MS by default, as this gives decent Unicode support
switch $tcl_platform(os) {
    "Darwin" -
    "windows" {minsky.defaultFont "Arial Unicode MS"}
}

# read in .rc file, which differs on Windows and unix
set rcfile ""
if {$tcl_platform(platform)=="unix"} {
  set rcfile "$env(HOME)/.minskyrc"
} elseif {$tcl_platform(platform)=="windows"} {
  set rcfile "$env(USERPROFILE)/.minskyrc"
}
if [file exists $rcfile] {
  catch {source $rcfile}
}

# for some reason (MingW I'm looking at you), tcl_library sometimes
# points to Minsky's library directory
if {![file exists [file join $tcl_library init.tcl]] || 
    [file normalize $tcl_library]==[file normalize $minskyHome/library]} {
    set tcl_library $minskyHome/library/tcl
}


# hopefully, we can now rely on TCL/Tk to find its own libraries.
if {![info exists tk_library]} {
  regsub tcl [file tail [info library]] tk tk_library
  set tk_library [file join [file dirname [info library]] $tk_library]
}
if {![file exists [file join $tk_library tk.tcl]]
} {
    set tk_library $minskyHome/library/tk
}

# In a rather bizarre set of events, namely Aqua build, launched via
# launch services, on a non-developer machine, Tk will instantiate a
# second TCL interpreter to host a console. Unfortunately, it uses the
# original heuristics to find the TCL and TK libraries, which fail,
# causing the application to hang. Setting these environment variables
# cuts through that problem. See ticket #675.
set env(TCL_LIBRARY) $tcl_library
set env(TK_LIBRARY) $tk_library

proc setFname {name} {
    global fname workDir progName
    if [string length $name] {
        set fname $name
        set workDir [file dirname $name]
        catch {wm title . "$progName: $fname"}
    }
}

# needed for scripts/tests
rename exit tcl_exit

# emulate minsky.value.value's default argument
proc value.value {args} {
    if [llength $args] {
        return [minsky.value.value [lindex $args 0]]
    } else {
        return [minsky.value.value 0]
    }
}
proc attachTraceProc {namesp} {
    foreach p [info commands $namesp*] {
        if {$p ne "::traceProc"} {
            trace remove execution $p enterstep traceProc
            trace add execution $p enterstep traceProc
        }
    }
    # recursively process child namespaces
    foreach n [namespace children $namesp] {
        attachTraceProc ${n}::
    }
}

proc traceProc {args} {
    array set frameInfo [info frame -2]
    if  {[info exists frameInfo(proc)]&&[info exists frameInfo(line)]} {
        cov.add $frameInfo(proc) $frameInfo(line)
    }
    if  {[info exists frameInfo(file)]&&[info exists frameInfo(line)]} {
        cov.add $frameInfo(file) $frameInfo(line)
    }
}
 
if [info exists env(MINSKY_COV)] {
    # open coverage database, and set cache size
    Coverage cov
    cov.init $env(MINSKY_COV) w
    cov.max_elem 10000
    rename tcl_exit tcl_exit2
    proc tcl_exit {args} {
        # disable coverage testing
        proc traceProc {args} {}
        cov.close
        eval tcl_exit2 $args
    }
#    attachTraceProc ::
}

#Needs to be present to allow callbacks to change the cursor for busy operations
proc setCursor {cur} {}

#if argv(1) has .tcl extension, it is a script, otherwise it is data
if {$argc>1 && [string match "*.tcl" $argv(1)]} {source $argv(1)}

source $tcl_library/init.tcl

if {$tcl_platform(os)=="Darwin"} {
    if {[catch {GUI}] || [catch {source [file join $tk_library tk.tcl]}]} {
    # pop a message box about installing XQuartz
    exec osascript << "tell application \"System Events\"
    activate
    display dialog \"GUI failed to initialise, try installing XQuartz\"
    end tell"
}
} else {
    GUI
    source [file join $tk_library tk.tcl]
}

source [file join $tk_library bgerror.tcl]
source $minskyHome/library/init.tcl
source $minskyHome/helpRefDb.tcl

disableEventProcessing

# Tk's implementation of bgerror does not mark the error dialog as
# transient, creating a usability problem where a user could hide the
# dialog, and wonder why the application is not responding.
rename ::tk::SetFocusGrab ::tk::SetFocusGrab_
proc ::tk::SetFocusGrab {grab focus} {
  ::tk::SetFocusGrab_ $grab $focus
  wm attributes $grab -topmost 1
  wm transient $grab .
}

catch {console hide}

proc setBackgroundColour bgc {
    global backgroundColour
    set backgroundColour $bgc
    tk_setPalette $bgc
    # tk_setPalette doesn't understand ttk widgets
    ttk::style configure TNotebook -background $backgroundColour
    ttk::style configure TNotebook.Tab -background $backgroundColour
    ttk::style map TNotebook.Tab -background "selected $bgc active $bgc"
    if [winfo exists .controls.runmode] {.controls.runmode configure -selectcolor $bgc}
}

# disable tear-off menus
option add *Menu.tearOff 0
wm deiconify .
tk appname [file rootname [file tail $argv(0)]]
wm title . "$progName: $fname" 
setBackgroundColour $backgroundColour
proc tk_focusPrev {win} {return $win}
proc tk_focusNext {win} {return $win}
if {$preferences(focusFollowsMouse)} {
    tk_focusFollowsMouse
# Make tab traversal possible within a window that is given focus by only clicking on it (no focusFollowsMouse). For ticket 901.	
} else {
    set old [bind all <Enter>]
    set script {
	if {"%d" eq "NotifyAncestor" || "%d" eq "NotifyNonlinear" \
		|| "%d" eq "NotifyInferior"} {
	       tk::FocusOK %W	
	    }
    }
    if {$old ne ""} {
	bind all <Enter> "$old; $script"
    } else {
	bind all <Enter> $script
    }
}
proc setCursor {cur} {. configure -cursor $cur; update idletasks}

if {[tk windowingsystem]=="win32"} {
    # redirect the mousewheel event to the actual window that should
    # receive the event - see ticket #114 
    bind . <MouseWheel> {
        switch [winfo containing %X %Y] {
            .wiring.canvas {
                if {%D>=0} {
                    # on Winblows, min val of |%D| is 120, so just use sign
                    zoomAt %x %y 1.1
                } {
                    zoomAt %x %y [expr 1.0/1.1]
                } 
            }
        }
    }
}

source $minskyHome/library/tooltip.tcl
namespace import tooltip::tooltip

source $minskyHome/library/obj-browser.tcl

# Macs have a weird numbering of mouse buttons, so lets virtualise B2 & B3
# see http://wiki.tcl.tk/14728
if {[tk windowingsystem] == "aqua"} {
    event add <<contextMenu>> <Button-2> <Control-Button-1>
    event add <<middleMouse>> <Button-3>
    event add <<middleMouse-Motion>> <B3-Motion>
    event add <<middleMouse-ButtonRelease>> <B3-ButtonRelease>
} else {
    event add <<contextMenu>> <Button-3>
    event add <<middleMouse>> <Button-2>
    event add <<middleMouse-Motion>> <B2-Motion>
    event add <<middleMouse-ButtonRelease>> <B2-ButtonRelease>
}

use_namespace minsky

proc deiconify {widget} {
    wm deiconify $widget
    after idle "wm deiconify $widget; raise $widget; focus -force $widget"
}

#labelframe .menubar -relief raised
menu .menubar -type menubar

if {[tk windowingsystem] == "aqua"} {
    menu .menubar.apple
    .menubar.apple add command -label "About Minsky" -command aboutMinsky
    .menubar add cascade -menu .menubar.apple
    set meta Command
    set meta_menu Cmd

# handle opening of files once Minsky is in flight
    proc ::tk::mac::OpenDocument {args} {
        # only support opening a single document at a time
        if {[llength $args]>0} {openNamedFile [lindex $args 0]}
    }
    proc ::tk::mac::ShowPreferences {} {showPreferences}
    proc ::tk::mac::ShowHelp {} {help Introduction}
} else {
    # keyboard accelerator introducer, which is different on macs
    set meta Control
    set meta_menu Ctrl
}

menu .menubar.file
.menubar add cascade -menu .menubar.file -label File -underline 0

menu .menubar.edit -postcommand togglePaste
.menubar add cascade -menu .menubar.edit -label Edit -underline 0

menu .menubar.bookmarks -postcommand generateBookmarkMenu
.menubar add cascade -menu .menubar.bookmarks -label Bookmarks -underline 0
menu .menubar.bookmarks.deleteMenu

menu .menubar.ops
.menubar add cascade -menu .menubar.ops -label Insert -underline 0

menu .menubar.options
.menubar add cascade -menu .menubar.options -label Options -underline 0
.menubar.options add command -label "Preferences" -command showPreferences

# add rows to this table to add new preferences
# valid types are "text", "bool" and "{ enum label1 val1 label2 val2 ... }"
#   varName              Label                    DefaultVal    type
set preferencesVars {
    godleyDisplay        "Godley Table Show Values"      1      bool
    godleyDisplayStyle       "Godley Table Output Style"    sign  { enum
        "DR/CR" DRCR
        "+/-" sign }       
    multipleEquities     "Enable multiple equity columns"      1      bool         
    nRecentFiles          "Number of recent files to display" 10 text
    wrapLaTeXLines        "Wrap long equations in LaTeX export" 1 bool
    panopticon        "Enable panopticon" 1 bool
    focusFollowsMouse        "Focus follows mouse" 1 bool    
}
lappend preferencesVars defaultFont "Font" [defaultFont] font

foreach {var text default type} $preferencesVars {
    # don't override value set in the rc file
    if {![info exists preferences($var)]} {
        set preferences($var) $default
    }
}
            
proc showPreferences {} {
    if [winfo exists .preferencesForm] return
    global preferences_input preferences preferencesVars
    foreach var [array names preferences] {
	set preferences_input($var) $preferences($var)
    }

    toplevel .preferencesForm
    wm resizable .preferencesForm 0 0
    
    set row 0
    
    grid [label .preferencesForm.label$row -text "Preferences"] -column 1 -columnspan 999 -pady 10
    incr row 10

    # pad the left and right
    grid [frame .preferencesForm.f1] -column 1 -row 1 -rowspan 999 -padx 10
    grid [frame .preferencesForm.f2] -column 999 -row 1 -rowspan 999 -padx 10
    

    foreach {var text default type} $preferencesVars {
        set rowdict($text) $row

        grid [label .preferencesForm.label$row -text $text] -column 10 -row $row -sticky e -pady 5
        
        switch $type {
            text {
                grid [entry  .preferencesForm.text$row -width 20 -textvariable preferences_input($var)] -column 20 -row $row -sticky ew -columnspan 999
            }
            bool {
                grid [checkbutton .preferencesForm.cb$row -variable preferences_input($var)] -row $row -column 20 -sticky w
            }
            font {
                grid [ttk::combobox .preferencesForm.font -textvariable preferences_input($var) -values [lsort [listFonts]] -state readonly] -row $row -column 20 -sticky w
                image create cairoSurface fontSampler -surface minsky.fontSampler
                grid [label .preferencesForm.fontSample -image fontSampler -width 150 -height 20] -row $row -column 30 -sticky w
                bind .preferencesForm.font <<ComboboxSelected>> {
                    defaultFont [.preferencesForm.font get]
                    fontSampler.requestRedraw
                    canvas.requestRedraw
                }
            }
            default {
                if {[llength $type] > 1} {
                    switch [lindex $type 0] {
                        enum {
                            set column 20
                            foreach {valtext val} [lrange $type 1 end] {
                                grid [radiobutton .preferencesForm.rb${row}v$column  -text $valtext -variable preferences_input($var) -value $val] -row $row -column $column
                                incr column 
                            }
                        }
                    }
                } else { error "unknown preferences widget $type" }
            }
        }
        
        incr row 10
    }
    
    set preferences(initial_focus) ".preferencesForm.cb$rowdict(Godley Table Show Values)"
    
    frame .preferencesForm.buttonBar
    button .preferencesForm.buttonBar.ok -text OK -command {setPreferenceParms; closePreferencesForm; redrawAllGodleyTables}
    button .preferencesForm.buttonBar.cancel -text cancel -command {closePreferencesForm}
    pack .preferencesForm.buttonBar.ok [label .preferencesForm.buttonBar.spacer -width 2] .preferencesForm.buttonBar.cancel -side left -pady 10
    grid .preferencesForm.buttonBar -column 1 -row 999 -columnspan 999
    
    bind .preferencesForm <Key-Return> {invokeOKorCancel .preferencesForm.buttonBar}

    wm title .preferencesForm "Preferences"

    deiconify .preferencesForm
    update idletasks
    ::tk::TabToWindow $preferences(initial_focus)
    ensureWindowVisible .preferencesForm
    grab set .preferencesForm
    wm transient .preferencesForm .
}

.menubar.options add command -label "Background Colour" -command {
    set bgc [tk_chooseColor -initialcolor $backgroundColour]
    if {$bgc!=""} {
        set backgroundColour $bgc
        setBackgroundColour $backgroundColour
    }
}

menu .menubar.rungeKutta
.menubar.rungeKutta add command -label "Simulation" -command {
    foreach {var text} $rkVars { set rkVarInput($var) [$var] }
    set implicitSolver [implicit]
    deiconifyRKDataForm
    update idletasks
    ::tk::TabToWindow $rkVarInput(initial_focus)
    ensureWindowVisible .rkDataForm
    grab set .rkDataForm
    wm transient .rkDataForm .
} -underline 0 
.menubar add cascade -label "Simulation" -menu .menubar.rungeKutta

# special platform specific menus
menu .menubar.help
if {[tk windowingsystem] != "aqua"} {
    .menubar.help add command -label "Minsky Documentation" -command {help Introduction}
}
.menubar add cascade -label Help -menu .menubar.help

bind . <F1> topLevelHelp
bind .menubar.file <F1> {help File}


# placement of menu items in menubar
. configure -menu .menubar 

# controls toolbar 

labelframe .controls
label .controls.statusbar -text "t: 0 Δt: 0"

# classic mode
set classicMode 0

if {$classicMode} {
    button .controls.run -text run -command runstop
    button .controls.reset -text reset -command reset
    button .controls.step -text step -command {step}
} else {
    image create photo runButton -file "$minskyHome/icons/Play.gif" 
    image create photo stopButton -file "$minskyHome/icons/Pause.gif"
    image create photo resetButton -file "$minskyHome/icons/Rewind.gif"
    image create photo stepButton -file "$minskyHome/icons/Last.gif"
    # iconic mode
    button .controls.run -image runButton -height 25 -width 25 -command runstop
    button .controls.reset -image resetButton -height 25 -width 25 -command reset
    button .controls.step -image stepButton -height 25 -width 25  -command {step}
}

image create photo recalculate -file "$minskyHome/icons/recalculate.gif"
button .controls.recalculate -image recalculate -height 25 -width 25  -command reset
image create photo rec -file "$minskyHome/icons/rec.gif"
image create photo runmode -file "$minskyHome/icons/runmode.gif"
image create photo recplay -file "$minskyHome/icons/recplay.gif"

checkbutton .controls.rec -image rec -height 25 -width 25 -command toggleRecording -variable eventRecording -indicatoron 0
checkbutton .controls.runmode -image runmode -height 25 -width 25 -selectimage recplay -variable recordingReplay -command replay -indicatoron 0 -selectcolor $backgroundColour
checkbutton .controls.reverse -text "Rev" -command {
    minsky.reverse $reverse} -variable reverse
    
tooltip .controls.recalculate "Recalculate"
tooltip .controls.rec "Record"
tooltip .controls.runmode "Simulate/Recording Replay"
tooltip .controls.reverse "Reverse simulation"
tooltip .controls.run "Run/Stop"
tooltip .controls.reset "Reset simulation"
tooltip .controls.step "Step simulation"

# enable auto-repeat on step button
bind .controls.step <ButtonPress-1> {set buttonPressed 1; autoRepeatButton .controls.step}
bind . <ButtonRelease-1> {set buttonPressed 0}

proc generateBookmarkMenu {} {
    .menubar.bookmarks delete 0 end
    .menubar.bookmarks add command -label "Bookmark this position" -command addBookMark
    .menubar.bookmarks.deleteMenu delete 0 end
    .menubar.bookmarks add cascade -label "Delete ..." -menu .menubar.bookmarks.deleteMenu
    .menubar.bookmarks add separator
    set p 0
    foreach i [minsky.canvas.model.bookmarkList] {
        .menubar.bookmarks add command -label $i -command "canvas.model.gotoBookmark $p; canvas.requestRedraw"
        .menubar.bookmarks.deleteMenu add command -label $i -command "canvas.model.deleteBookmark $p"
        incr p
    }
}

proc addBookMark {} {
    toplevel .bookMarkDialog
    frame .bookMarkDialog.name
    label .bookMarkDialog.name.text -text "Bookmark name"
    entry .bookMarkDialog.name.val
    pack .bookMarkDialog.name.text .bookMarkDialog.name.val -side left
    pack .bookMarkDialog.name
    buttonBar .bookMarkDialog {
        minsky.canvas.model.addBookmark [.bookMarkDialog.name.val get]
    }
    ensureWindowVisible .bookMarkDialog
    wm transient .bookMarkDialog
    focus .bookMarkDialog.name.val
    grab set .bookMarkDialog
}
    
# self submitting script that continues while a button is pressed
proc invokeButton button {
    global buttonPressed
    if {$buttonPressed} {
        $button invoke
        update
        autoRepeatButton $button
    }
}

proc autoRepeatButton button {
    after 500 invokeButton $button
}

proc setSimulationDelay {delay} {
    # on loading a model, slider is adjusted, which causes
    # simulationDelay to be set unnecessarily, marking the model
    # dirty
    if {$delay != [simulationDelay]} {
        pushFlags
        simulationDelay $delay
        popFlags
    }
}

label .controls.slowSpeed -text "slow"
label .controls.fastSpeed -text "fast"
scale .controls.simSpeed -variable delay -command setSimulationDelay -to 0 -from 12 -length 150 -label "Simulation Speed" -orient horizontal -showvalue 0



pack .controls.recalculate .controls.rec .controls.runmode .controls.reverse .controls.run .controls.reset .controls.step .controls.slowSpeed .controls.simSpeed .controls.fastSpeed -side left
pack .controls.statusbar -side right -fill x

grid .controls -row 0 -column 0 -columnspan 1000 -sticky ew

menu .menubar.file.recent

menu .exportPlots
.exportPlots add command -label "as SVG" -command {minsky.renderAllPlotsAsSVG [file rootname $fname]}
.exportPlots add command -label "as CSV" -command {minsky.exportAllPlotsAsCSV [file rootname $fname]}

# File menu
.menubar.file add command -label "About Minsky" -command aboutMinsky
.menubar.file add command -label "Upgrade" -command {openURL http://www.patreon.com/hpcoder}
.menubar.file add command -label "New System" -command newSystem  -underline 0 -accelerator $meta_menu-N
.menubar.file add command -label "Open" -command openFile -underline 0 -accelerator $meta_menu-O
.menubar.file add cascade -label "Recent Files"  -menu .menubar.file.recent
.menubar.file add command -label "Library"  -command "openURL https://github.com/highperformancecoder/minsky-models"

.menubar.file add command -label "Save" -command save -underline 0 -accelerator $meta_menu-S
.menubar.file add command -label "SaveAs" -command saveAs -underline 4 -accelerator $meta_menu-A 
.menubar.file add command -label "Insert File as Group" -command insertFile
.menubar.file add command -label "Import Vensim MDL file" -command importMDL

.menubar.file add command -label "Dimensional Analysis" -command {
    dimensionalAnalysis
    tk_messageBox -type ok -icon info -message "Dimension Analysis passed"
}

.menubar.file add command -label "Export resolution factor: [minsky.canvas.resolutionScaleFactor]" -command {setExportResolutionFactor minsky.canvas}
.menubar.file add command -label "Export Canvas" -command exportCanvas
.menubar.file add cascade -label "Export Plots" -menu .exportPlots
.menubar.file add checkbutton -label "Log simulation" -variable simLogging \
    -command getLogVars
.menubar.file add checkbutton -label "Recording" -command toggleRecording -variable eventRecording
.menubar.file add checkbutton -label "Replay recording" -command replay -variable recordingReplay 
    
.menubar.file add command -label "Quit" -command exit -underline 0 -accelerator $meta_menu-Q
.menubar.file add separator
.menubar.file add command  -foreground #5f5f5f -label "Debugging Use"
.menubar.file add command -label "Redraw" -command canvas.requestRedraw
.menubar.file add command -label "Object Browser" -command obj_browser
.menubar.file add command -label "Select items" -command selectItems
.menubar.file add command -label "Command" -command cli

.menubar.file configure -postcommand {
    .menubar.file entryconfigure "Export resolution factor:*" -label "Export resolution factor: [minsky.canvas.resolutionScaleFactor]"
}

proc imageFileTypes {} {
    global tcl_platform
    set fileTypes {{"SVG" .svg TEXT} {"PDF" .pdf TEXT} {"Postscript" .eps TEXT} {"Portable Network Graphics" .png TEXT}}
    if {$tcl_platform(platform)=="windows"} {lappend fileTypes {"EMF" .emf TEXT}}
    return $fileTypes
}

proc setExportResolutionFactor {setter} {
    toplevel .resolutionFactor
    wm title .resolutionFactor "Export resolution scale factor"
    ttk::spinbox .resolutionFactor.value -from 1 -to 1000 -increment 1
    pack .resolutionFactor.value
    .resolutionFactor.value set [$setter.resolutionScaleFactor]
    buttonBar .resolutionFactor "$setter.resolutionScaleFactor \[.resolutionFactor.value get\]"
}


proc renderImage {filename type surf} {
    global tcl_platform
    if [string match -nocase *.svg "$filename"] {
        $surf.renderToSVG "$filename"
    } elseif [string match -nocase *.pdf "$filename"] {
        $surf.renderToPDF "$filename"
    } elseif {[string match -nocase *.ps "$filename"] || [string match -nocase *.eps "$filename"]} {
        $surf.renderToPS "$filename"
    } elseif {[string match -nocase *.png "$filename"]} {
        $surf.renderToPNG "$filename"
    } elseif {$tcl_platform(platform)=="windows" && [string match -nocase *.emf "$filename"]} {
        $surf.renderToEMF "$filename"
    } else {
        switch $type {
            "SVG" {$surf.renderToSVG  "$filename.svg"}
            "PDF" {$surf.renderToPDF "$filename.pdf"}
            "PNG" {$surf.renderToPNG "$filename.png"}
            "EMF" {$surf.renderToEMF "$filename.emf"}
            "Postscript" {$surf.renderToPS "$filename.eps"}
            default {return false}
        }
    }
    return true
}

proc exportCanvas {} {
    global workDir type fname preferences tabSurface

    set fileTypes [imageFileTypes]
    lappend fileTypes {"LaTeX" .tex TEXT} {"Matlab" .m TEXT}
    set f [tk_getSaveFile -filetypes $fileTypes \
               -initialdir $workDir -typevariable type -initialfile [file rootname [file tail $fname]]]  
    if {$f==""} return
    set workDir [file dirname $f]
    # extract the surface name from the current tab, for #912
    set surf [lindex [.tabs tabs] [.tabs index current]].canvas
    if [renderImage $f $type $tabSurface([.tabs tab current -text])] return
    if {[string match -nocase *.tex "$f"]} {
        latex "$f" $preferences(wrapLaTeXLines)
    } elseif {[string match -nocase *.m "$f"]} {
        matlab "$f"
    } else {
        switch $type {
            "LaTeX" {latex "$f.tex" $preferences(wrapLaTeXLines)}
            "Matlab" {matlab "$f.m"}
        }
    }
}


proc getLogVars {} {
    global varNames allLogVars varIds
    set varNames {}
    set varIds {}
    set allLogVars 0
    toplevel .logVars
    frame .logVars.buttons
    button .logVars.buttons.ok -text OK -command logVarsOK
    checkbutton .logVars.buttons.all -text All -variable allLogVars -command {
        .logVars.selection.selection selection [expr $allLogVars?"set":"clear"] 0 end
    }            
    pack .logVars.buttons.ok .logVars.buttons.all -side left

    foreach v [variableValues.#keys] {
        if {![regexp "^constant:" $v]} {
            getValue $v
            lappend varNames [minsky.value.name]
            lappend varIds $v
        }
    }
    frame .logVars.selection
    listbox .logVars.selection.selection -listvariable varNames -selectmode extended -height 30 -yscrollcommand ".logVars.selection.vscroll set" -selectforeground blue
    scrollbar .logVars.selection.vscroll -orient vertical -command ".logVars.selection.selection yview"
    pack .logVars.selection.selection -fill both -side left -expand y
    pack .logVars.selection.vscroll -fill y -side left -expand y
    pack .logVars.buttons .logVars.selection
    
    ensureWindowVisible .logVars
    grab set .logVars
    wm transient .logVars
}

proc logVarsOK {} {
    global workDir varIds
    set indices [.logVars.selection.selection curselection]
    
    foreach i $indices {lappend vars [lindex $varIds $i]}
    logVarList $vars
    destroy .logVars
    openLogFile [tk_getSaveFile -defaultextension .dat -initialdir $workDir]
}




.menubar.edit add command -label "Undo" -command "undo 1" -accelerator $meta_menu-Z
.menubar.edit add command -label "Redo" -command "undo -1" -accelerator $meta_menu-Y
.menubar.edit add command -label "Cut" -command cut -accelerator $meta_menu-X
.menubar.edit add command -label "Copy" -command "minsky.copy" -accelerator $meta_menu-C
.menubar.edit add command -label "Paste" -command "minsky.paste" -accelerator $meta_menu-V
.menubar.edit add command -label "Group selection" -command "minsky.createGroup" -accelerator $meta_menu-G
.menubar.edit add command -label "Dimensions" -command dimensionsDialog
.menubar.edit add command -label "Remove units" -command minsky.deleteAllUnits
.menubar.edit add command -label "Randomize layout" -command minsky.randomLayout
.menubar.edit add command -label "Auto layout" -command minsky.autoLayout

proc getClipboard {} {
    set contents ""
    catch {clipboard get -type UTF8_STRING} contents
    return contents
}

proc getClipboard {} {
    set contents ""
    catch {clipboard get -type UTF8_STRING} contents
    return contents
}

proc togglePaste {} {
    if {[getClipboard]==""} {
	.menubar.edit entryconfigure "Paste" -state disabled
    } else {
	.menubar.edit entryconfigure "Paste" -state normal
    }
}

proc undo {delta} {
    # do not record changes to state from the undo command
    doPushHistory 0
    minsky.undo $delta
    minsky.canvas.requestRedraw
    deleteSubsidiaryTopLevels
    doPushHistory 1
}

proc cut {} {
    minsky.cut
}

proc dimensionsDialog {} {
    populateMissingDimensions
    toplevel .dimensions
    grid [button .dimensions.cancel -text Cancel -command "destroy .dimensions"] \
        [button .dimensions.ok -text OK -command {
            set colRows [grid size .dimensions]
            for {set i 2} {$i<[lindex $colRows 1]} {incr i} {
                set dim [.dimensions.g${i}_dim get]
                if {$dim!=""} {
                    set d [dimensions.@elem $dim]
                            $d.type [.dimensions.g${i}_type get]
                            if [info exists timeFormatStrings([.dimensions.g${i}_units get])] {
                                $d.units $timeFormatStrings([.dimensions.g${i}_units get])
                            } else {
                                $d.units [.dimensions.g${i}_units get]
                            }
                        }
                    }
            imposeDimensions
            destroy .dimensions
            reset
        }]
    grid [label .dimensions.g1_dim -text Dimension] \
        [label .dimensions.g1_type -text Type]\
        [label .dimensions.g1_units -text "Units/Format"]
    tooltip .dimensions.g1_units "Value type: enter a unit string, eg m/s; time type: enter a strftime format string, eg %Y-%m-%d %H:%M:%S, or %Y-Q%Q"

    set colRows [grid size .dimensions]
    for {set i [lindex $colRows 1]} {$i<[dimensions.size]+3} {incr i} {
        grid [entry .dimensions.g${i}_dim] \
            [ttk::combobox .dimensions.g${i}_type -state readonly \
                 -values {string value time}] \
            [ttk::combobox .dimensions.g${i}_units \
                 -postcommand "dimFormatPopdown .dimensions.g${i}_units \[.dimensions.g${i}_type get\] {}"
            ]
    }
    set i 2
    foreach dim [dimensions.#keys] {
        set d [dimensions.@elem $dim]
        .dimensions.g${i}_dim delete 0 end
        .dimensions.g${i}_dim insert 0 $dim
        .dimensions.g${i}_type set [$d.type]
        .dimensions.g${i}_units delete 0 end
        .dimensions.g${i}_units insert 0 [$d.units]
        dimFormatPopdown .dimensions.g${i}_units [$d.type] {}
        incr i
    }
}



array set timeFormatStrings {
    "1999-Q4" "%Y-Q%Q"
    "1999" "%Y"
    "12/31/99" "%m/%d/%y"
    "12/31/1999" "%m/%d/%Y"
    "31/12/99" "%d/%m/%y"
    "31/12/1999" "%d/%m/%Y"
    "1999-12-31T13:37:46" "%Y-%m-%dT%H:%M:%S"
    "12/31/1999 01:37 PM" "%m/%d/%Y %I:%M %p"
    "12/31/99 01:37 PM" "%m/%d/%y %I:%M %p"
    "12/31/1999 13:37 PM" "%m/%d/%Y %H:%M %p"
    "12/31/99 13:37 PM" "%m/%d/%y %H:%M %p"
    "Friday, December 31, 1999" "%A, %B %d, %Y"
    "Dec 31, 99" "%b %d, %y"
    "Dec 31, 1999" "%b %d, %Y"
    "31. Dec. 1999" "%d. %b. %Y"
    "December 31, 1999" "%B %d, %Y"
    "31. December 1999" "%d. %B %Y"
    "Fri, Dec 31, 99" "%a, %b %d, %y"
    "Fri 31/Dec 99" "%a %d/%b %y"
    "Fri, Dec 31, 1999" "%a, %b %d, %Y"
    "Friday, December 31, 1999" "%A, %B %d, %Y"
    "12-31" "%m-%d"
    "99-12-31" "%y-%m-%d"
    "1999-12-31" "%Y-%m-%d"
    "12/99" "%m/%y"
    "Dec 31" "%b %d"
    "December" "%B"
    "4th quarter 99" "%Qth quarter %y"
}

proc rewriteTimeComboBox {comboBox} {
    global timeFormatStrings
    if [info exists timeFormatStrings([$comboBox get])] {
        $comboBox set $timeFormatStrings([$comboBox get])
    }
}

# If comboBox is a format combo box for a field of \a type, then set up rewrite strings, then execute \a onSelect
proc dimFormatPopdown {comboBox type onSelect} {
    global timeFormatStrings
    switch $type {
        string {
            $comboBox configure -values {}
            $comboBox set {}
            bind $comboBox <<ComboboxSelected>> $onSelect
        }
        value {
            $comboBox configure -values {}
            bind $comboBox <<ComboboxSelected>> $onSelect
        }
        time {
            $comboBox configure -values [lsort [array names timeFormatStrings]]
            bind $comboBox <<ComboboxSelected>> "rewriteTimeComboBox $comboBox; $onSelect"
        }
    }
}

proc pasteAt {} {
    minsky.paste
    canvas.mouseMove [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]
}

wm protocol . WM_DELETE_WINDOW exit
# keyboard accelerators
bind . <$meta-s> save
bind . <$meta-S> save
bind . <$meta-a> saveAs
bind . <$meta-A> saveAs
bind . <$meta-o> openFile
bind . <$meta-O> openFile
bind . <$meta-n> newSystem
bind . <$meta-N> newSystem
bind . <$meta-q> exit
bind . <$meta-Q> exit
bind . <$meta-y> "undo -1"
bind . <$meta-Y> "undo -1"
bind . <$meta-z> "undo 1"
bind . <$meta-Z> "undo 1"
bind . <$meta-x> {minsky.cut}
bind . <$meta-X> {minsky.cut}
bind . <$meta-c> {minsky.copy}
bind . <$meta-C> {minsky.copy}
bind . <$meta-v> {pasteAt}
bind . <$meta-V> {pasteAt}
bind . <$meta-g> {minsky.createGroup}
bind . <$meta-G> {minsky.createGroup}

# tabbed manager
ttk::notebook .tabs -padding 0
ttk::notebook::enableTraversal .tabs
# disable arrow bindings for switching between tabs, as we want to use these on the canvas
bind .tabs <Left> {}
bind .tabs <Right> {}
grid .tabs -column 0 -row 10 -sticky news
grid columnconfigure . 0 -weight 1
grid rowconfigure . 10 -weight 1

# utility for creating OK/Cancel button bar
proc buttonBar {window okProc} {
    frame $window.buttonBar
    button $window.buttonBar.ok -text "OK" -command "okAction \{$okProc\} $window"
    button $window.buttonBar.cancel -text "Cancel" -command "cancelWin $window"
    pack $window.buttonBar.cancel $window.buttonBar.ok -side left
    pack $window.buttonBar -side top
    bind $window <Key-Return> "$window.buttonBar.ok invoke"
    bind $window <Key-Escape> "$window.buttonBar.cancel invoke"
}

proc okAction {okProc window} {
    if [catch $okProc msg] {
        tk_messageBox  -icon error -parent $window -message $msg
        raise $window
        return
    }
    cancelWin $window
}

proc cancelWin window {
    grab release $window
    destroy $window
}

proc ensureWindowVisible window {
    if {![winfo ismapped $window]} {
        tkwait visibility $window
    }
}

# pop up a text entry widget to capture some user input
# @param win is top level window name
# @param init initialises the entry widget
# @param okproc gets executed when OK pressed. Use [$win.entry get] to return user value
proc textEntryPopup {win init okproc} {
    if {![winfo exists $win]} {
        toplevel $win
        entry $win.entry
        pack $win.entry -side top -ipadx 50
        buttonBar $win $okproc
    } else {
        wm deiconify $win
    }
    $win.entry delete 0 end
    $win.entry insert 0 $init
    wm transient $win
    focus $win.entry
    ensureWindowVisible $win
    grab set $win
    
}

bind .tabs <<contextMenu>> {
    set windows [.tabs tabs]
    set idx [.tabs identify tab %x %y]
    if {$idx<[llength $windows]} {
        .wiring.context delete 0 end
        .wiring.context add command -label Help -command "
            help $helpTopics([lindex $windows $idx])"
        tk_popup .wiring.context %X %Y
    }
}

proc addTab {window label surface} {
    image create cairoSurface rendered$window -surface $surface
    ttk::frame .$window
    global canvasHeight canvasWidth tabSurface helpTopics
    label .$window.canvas -image rendered$window -height $canvasHeight -width $canvasWidth
    .tabs add .$window -text $label -padding 0
    set tabSurface($label) $surface
    set helpTopics(.$window) tabs:$label
}

# add the tabbed windows
addTab wiring "Wiring" minsky.canvas
addTab equations "Equations" minsky.equationDisplay
pack .equations.canvas -fill both -expand 1

.tabs select 0

proc hoverMouseTab {tabId} {
    set tab [lindex [.tabs tabs] [.tabs index current]]
    $tabId.displayDelayedTooltip [get_pointer_x $tab.canvas] [get_pointer_y $tab.canvas]
}

# reset hoverMouse timer
proc wrapHoverMouseTab {tabId op x y} {
    after cancel hoverMouseTab $tabId
    # ignore any exceptions
    catch {$tabId.$op $x $y}
    after 3000 hoverMouseTab $tabId
}

proc tabContext {x y X Y} {
	    switch [lindex [.tabs tabs] [.tabs index current]] {
		.variables {	
		    .variables.context delete 0 end	
		    set r [variableTab.rowY $y]    	
		    switch [variableTab.clickType $x $y] {	
		        background {}	
		        internal {	
		    		set varName [variableTab.getVarName $r]	
		    		.variables.context add command -label "Remove $varName from tab" -command "variableTab.toggleVarDisplay $r;  variableTab.requestRedraw"	
		    	}	
		    }	
		    tk_popup .variables.context $X $Y	
	    }
	    .plts {
			.plts.context delete 0 end
			if [getPlotTabItemAt $x $y] {
				.plts.context add command -label "Remove plot from tab" -command "plotTab.togglePlotDisplay;  plotTab.requestRedraw"
			}
            tk_popup .plts.context $X $Y
		}		
	}
}

source $minskyHome/godley.tcl
source $minskyHome/plots.tcl
source $minskyHome/group.tcl
source $minskyHome/wiring.tcl
source $minskyHome/csvImport.tcl
source $minskyHome/ravel.tcl
source $minskyHome/variablePane.tcl

pack .wiring.canvas -fill both -expand 1

proc setScrollBars {} {
    switch [lindex [.tabs tabs] [.tabs index current]] {
        .wiring {
            set x0 [expr (10000-[minsky.canvas.model.x])/20000.0]
            set y0 [expr (10000-[minsky.canvas.model.y])/20000.0]
            .hscroll set $x0 [expr $x0+[winfo width .wiring.canvas]/20000.0]
            .vscroll set $y0 [expr $y0+[winfo height .wiring.canvas]/20000.0]
        }
        .equations {
            if {[equationDisplay.width]>0} {
                set x0 [expr [equationDisplay.offsx]/[equationDisplay.width]]
                .hscroll set $x0 [expr $x0+[winfo width .wiring.canvas]/[equationDisplay.width]]
            } else {.hscroll set 0 1}
            if {[equationDisplay.height]>0} {
                set y0 [expr [equationDisplay.offsx]/[equationDisplay.height]]
                .vscroll set $y0 [expr $y0+[winfo height .wiring.canvas]/[equationDisplay.height]]
            } else {.vscroll set  0 1}
        }
        .parameters {
            set x0 [expr (10000-[parameterTab.offsx])/20000.0]
            set y0 [expr (10000-[parameterTab.offsy])/20000.0]       
            .hscroll set $x0 [expr $x0+[winfo width .parameters.canvas]/20000.0]
            .vscroll set $y0 [expr $y0+[winfo height .parameters.canvas]/20000.0]           
		}      
        .variables {
            set x0 [expr (10000-[variableTab.offsx])/20000.0]
            set y0 [expr (10000-[variableTab.offsy])/20000.0]
            .hscroll set $x0 [expr $x0+[winfo width .variables.canvas]/20000.0]
            .vscroll set $y0 [expr $y0+[winfo height .variables.canvas]/20000.0]                 
        }
        .plts {
            set x0 [expr (10000-[plotTab.offsx])/20000.0]
            set y0 [expr (10000-[plotTab.offsy])/20000.0]
            .hscroll set $x0 [expr $x0+[winfo width .plts.canvas]/20000.0]
            .vscroll set $y0 [expr $y0+[winfo height .plts.canvas]/20000.0]           
        }  
        .gdlys {
            set x0 [expr (10000-[godleyTab.offsx])/20000.0]
            set y0 [expr (10000-[godleyTab.offsy])/20000.0]
            .hscroll set $x0 [expr $x0+[winfo width .gdlys.canvas]/20000.0]
            .vscroll set $y0 [expr $y0+[winfo height .gdlys.canvas]/20000.0]           
        }                         
    }
}

bind .tabs <<NotebookTabChanged>> {setScrollBars}

proc panCanvas {offsx offsy} {
    global preferences
    switch [lindex [.tabs tabs] [.tabs index current]] {
        .wiring {
            minsky.canvas.model.moveTo $offsx $offsy
            canvas.requestRedraw
        }
        .equations {
            equationDisplay.offsx $offsx
            equationDisplay.offsy $offsy
            equationDisplay.requestRedraw
        }
        .parameters {
            parameterTab.offsx $offsx
            parameterTab.offsy $offsy			
            parameterTab.requestRedraw
        }        
        .variables {
            variableTab.offsx $offsx
            variableTab.offsy $offsy						
            variableTab.requestRedraw
        }           
        .plts {
            plotTab.offsx $offsx
            plotTab.offsy $offsy	
            plotTab.requestRedraw
        }         
        .gdlys {
            godleyTab.offsx $offsx
            godleyTab.offsy $offsy	
            godleyTab.requestRedraw
        }             
    }
    setScrollBars
}


ttk::sizegrip .sizegrip
proc scrollCanvases {xyview args} {
    set win [lindex [.tabs tabs] [.tabs index current]]
    set ww [winfo width $win]
    set wh [winfo height $win]
    switch $win {
        .wiring {
            set x [minsky.canvas.model.x]
            set y [minsky.canvas.model.y]
            set w [expr 10*$ww]
            set h [expr 10*$wh]
            set x1 [expr 0.5*$w]
            set y1 [expr 0.5*$h]
        }
        .equations {
            set x [equationDisplay.offsx]
            set y [equationDisplay.offsy]
            set x1 0
            set y1 0
            set w [equationDisplay.width]
            set h [equationDisplay.height]
        }
        .parameters {
            set x [parameterTab.offsx]
            set y [parameterTab.offsy]
            set w [expr 10*$ww]
            set h [expr 10*$wh]
            set x1 [expr 0.5*$w]
            set y1 [expr 0.5*$h]       
        }
        .variables {
            set x [variableTab.offsx]
            set y [variableTab.offsy]
            set w [expr 10*$ww]
            set h [expr 10*$wh]
            set x1 [expr 0.5*$w]
            set y1 [expr 0.5*$h]
        }                
        .plts {
            set x [plotTab.offsx]
            set y [plotTab.offsy]
            set w [expr 10*$ww]
            set h [expr 10*$wh]
            set x1 [expr 0.5*$w]
            set y1 [expr 0.5*$h]
        }
        .gdlys {
            set x [godleyTab.offsx]
            set y [godleyTab.offsy]
            set w [expr 10*$ww]
            set h [expr 10*$wh]
            set x1 [expr 0.5*$w]
            set y1 [expr 0.5*$h]
        }                      
    }
    switch [lindex $args 0] {
        moveto {
            switch $xyview {
                xview {panCanvas [expr $x1-$w*[lindex $args 1]] $y}
                yview {panCanvas $x [expr $y1-$h*[lindex $args 1]]}
            }
        }
        scroll {
            switch [lindex $args 2] {
                units {set incr [expr [lindex $args 1]*0.01]}
                # page corresponds to one full screens worth
                pages {set incr [expr [lindex $args 1]*0.1]}
            }
            switch $xyview {
                xview {panCanvas [expr $x-$incr*$w] $y}
                yview {panCanvas $x [expr $y-$incr*$h]}
            }
        }
    }
}
scrollbar .vscroll -orient vertical -command "scrollCanvases yview"
scrollbar .hscroll -orient horiz -command "scrollCanvases xview"
update
setScrollBars

bind . <Key-Prior> {scrollCanvases yview scroll -1 pages}
bind . <Key-Next> {scrollCanvases yview scroll 1 pages}
bind . <Key-Home> {scrollCanvases xview scroll -1 pages}
bind . <Key-End> {scrollCanvases xview scroll 1 pages}

# adjust cursor for pan mode
if {[tk windowingsystem] == "aqua"} {
    set panIcon closedhand
} else {
    set panIcon fleur
}



# equations pan mode
.equations.canvas configure -cursor $panIcon
bind .equations.canvas <Button-1> {
    set panOffsX [expr %x-[equationDisplay.offsx]]
    set panOffsY [expr %y-[equationDisplay.offsy]]
}
bind .equations.canvas <B1-Motion> {panCanvas [expr %x-$panOffsX] [expr %y-$panOffsY]}

grid .sizegrip -row 999 -column 999
grid .vscroll -column 999 -row 10 -rowspan 989 -sticky ns
grid .hscroll -row 999 -column 0 -columnspan 999 -sticky ew

image create photo zoomOutImg -file $minskyHome/icons/zoomOut.gif
button .controls.zoomOut -image zoomOutImg -height 24 -width 37 \
    -command {zoom 0.91}
tooltip .controls.zoomOut "Zoom Out"

image create photo zoomInImg -file $minskyHome/icons/zoomIn.gif
button .controls.zoomIn -image zoomInImg -height 24 -width 37 \
    -command {zoom 1.1}
tooltip .controls.zoomIn "Zoom In"

image create photo zoomOrigImg -file $minskyHome/icons/zoomOrig.gif
button .controls.zoomOrig -image zoomOrigImg -height 24 -width 37 \
    -command {
        if {[minsky.model.zoomFactor]>0} {
            zoom [expr 1/[minsky.model.relZoom]]
        } else {
            minsky.model.setZoom 1
        }
        recentreCanvas
    }
tooltip .controls.zoomOrig "Reset Zoom/Origin"

image create photo zoomFitImg -file $minskyHome/icons/zoomFit.gif
button .controls.zoomFit -image zoomFitImg -height 24 -width 37 \
    -command {
        set cb [minsky.canvas.model.cBounds]
        set z1 [expr double([winfo width .wiring.canvas])/([lindex $cb 2]-[lindex $cb 0])]
        set z2 [expr double([winfo height .wiring.canvas])/([lindex $cb 3]-[lindex $cb 1])]
        if {$z2<$z1} {set z1 $z2}
        set x [expr -0.5*([lindex $cb 2]+[lindex $cb 0])]
        set y [expr -0.5*([lindex $cb 3]+[lindex $cb 1])]
        zoomAt $x $y $z1
        recentreCanvas
    }
tooltip .controls.zoomFit "Zoom to fit"
pack .controls.zoomOut .controls.zoomIn .controls.zoomOrig .controls.zoomFit -side left

set delay [simulationDelay]

proc runstop {} {
    global classicMode
    if [running] {
        running 0
        doPushHistory 1
        if {$classicMode} {
            .controls.run configure -text run
        } else {
            .controls.run configure -image runButton
        }
    } else {
        running 1
        doPushHistory 0
        if {$classicMode} {
            .controls.run configure -text stop
        } else {
            .controls.run configure -image stopButton
        }
        step
        simulate
    }
}

proc step {} {
    global recordingReplay eventRecordR simTMax simTStart
    if {$recordingReplay} {
        if {[gets $eventRecordR cmd]>=0} {
            eval $cmd
            update
        } else {
            runstop
        }
    } else {
        # run simulation
        global preferences
        if {[catch minsky.step errMsg options] && [running]} {runstop}
        if {[minsky.t0]>[t] || [minsky.tmax]<[t]} {runstop}
        .controls.statusbar configure -text "t: [t] Δt: [format %g [deltaT]]"
        if $preferences(godleyDisplay) redrawAllGodleyTables
        update
        return -options $options $errMsg
    }
}


proc simulate {} {
    uplevel #0 {
        if [running] {
              set d [expr int(pow(10,$delay/4.0))]
              after $d {
                  if [running] {
                      step
                      simulate
                  }
              }
        }
    }
}

proc reset {} {
    global recordingReplay eventRecordR simLogging eventRecording
    if {$eventRecording} {
        set eventRecording 0
        stopRecording
        return
    }
    running 0
    if {$recordingReplay} {
        seek $eventRecordR 0 start
        model.clear
        canvas.requestRedraw
    } else {
        set tstep 0
        set simLogging 0
        closeLogFile
        # delay throwing exception to allow display to be updated
        set err [catch minsky.reset result]
        .controls.statusbar configure -text "t: 0 Δt: 0"
        .controls.run configure -image runButton

        redrawAllGodleyTables
        return -code $err $result
    }
}



proc populateRecentFiles {} {
    global recentFiles preferences
    .menubar.file.recent delete 0 end
    if {[llength $recentFiles]>$preferences(nRecentFiles)} {
        set recentFiles [lreplace $recentFiles $preferences(nRecentFiles) end]
    }
    foreach f $recentFiles {
        .menubar.file.recent insert 0 command -label "[file tail $f]" \
            -command "openNamedFile \"[regsub -all {\\} $f /]\""
    }
}
populateRecentFiles

# load/save 

proc openFile {} {
    global fname workDir preferences
    set ofname [tk_getOpenFile -multiple 1 -filetypes {
        {Minsky {.mky}} {Ravel {.rvl}} {XML {.xml}} {All {.*}}} -initialdir $workDir]
    if [string length $ofname] {eval openNamedFile $ofname}
}

proc autoBackupName {} {
    global fname
    return "$fname#"
}
proc openNamedFile {ofname} {
    global fname workDir preferences
    newSystem
    setFname $ofname
    
    if {[file exists [autoBackupName]] && [tk_messageBox -message "Auto save file exists, do you wish to load it" -type yesno]=="yes"} {
        eval minsky.load {[autoBackupName]}
    } else {
        eval minsky.load {$ofname}
        file delete -- [autoBackupName]
    }
    # setting simulationDelay causes the edited (dirty) flag to be set, amongst other things
    pushFlags
    doPushHistory 0
    setAutoSaveFile [autoBackupName]

    # minsky.load resets minsky.multipleEquities and other preference, so restore preferences
    minsky.multipleEquities $preferences(multipleEquities)
    setGodleyDisplayValue $preferences(godleyDisplay) $preferences(godleyDisplayStyle)

    recentreCanvas

   .controls.simSpeed set [simulationDelay]
    # force update canvas size to ensure model is displayed correctly
    update
    canvas.requestRedraw
    # not sure why this is needed, but initial draw doesn't happen without it
    event generate .wiring.canvas <Expose>
    update
    doPushHistory 1
    pushHistory
    popFlags
}

proc insertFile {} {
    global workDir
    set fname [tk_getOpenFile -multiple 1 -filetypes {
        {Minsky {.mky}} {Ravel {.rvl}} {XML {.xml}} {All {.*}}} -initialdir $workDir]
    eval insertGroupFromFile $fname
}

proc importMDL {} {
    global workDir
    newSystem
    set fname [tk_getOpenFile -multiple 1 -filetypes {
        {Vensim {.mdl}} {All {.*}}} -initialdir $workDir]
    eval importVensim $fname
    minsky.model.autoLayout
}

# adjust canvas so that -ve coordinates appear on canvas
proc recentreCanvas {} {
    switch [lindex [.tabs tabs] [.tabs index current]] {
        .wiring {canvas.recentre}
        .equations {
            equationDisplay.offsx 0
            equationDisplay.offsy 0
            equationDisplay.requestRedraw
        }
        .parameters {
            parameterTab.offsx 0
            parameterTab.offsy 0
            parameterTab.requestRedraw
        }
        .variables {
            variableTab.offsx 0
            variableTab.offsy 0
            variableTab.requestRedraw
        }       
        .plts {
            plotTab.offsx 0
            plotTab.offsy 0
            plotTab.requestRedraw
        }               
        .gdlys {
            godleyTab.offsx 0
            godleyTab.offsy 0
            godleyTab.requestRedraw
        }                    
    }
}

proc fileTypes {defaultExtension} {
    if {$defaultExtension==".rvl"} {
        return {{"Ravel" .rvl TEXT} {"Minsky" .mky TEXT} {"All Files" * TEXT}}
    } else {
        return {{"Minsky" .mky TEXT} {"Ravel" .rvl TEXT} {"All Files" * TEXT}}
    }
}

proc save {} {
    global fname workDir
    set ext [minsky.model.defaultExtension]
    if {![string length $fname]} {
        setFname [tk_getSaveFile -defaultextension $ext  -initialdir $workDir \
                      -filetypes [fileTypes $ext]]}            
    if [string length $fname] {
        set workDir [file dirname $fname]
        eval minsky.save {$fname}
        file delete -- [autoBackupName]
    }
}

proc saveAs {} {
    global fname workDir
    set ext [minsky.model.defaultExtension]
    setFname [tk_getSaveFile -defaultextension $ext  -initialdir $workDir \
                  -filetypes [fileTypes $ext]]            
    if [string length $fname] save
}

proc newSystem {} {
    doPushHistory 0
    if {[edited]} {
        switch [tk_messageBox -message "Save?" -type yesnocancel] {
            yes save
            no {}
            cancel {return -level [info level]}
        }
    }    
    catch {reset}
    clearAllMapsTCL
    pushFlags
    deleteSubsidiaryTopLevels
    clearHistory
    model.setZoom 1
    recentreCanvas
    global fname progName
    set fname ""
    file delete [autoBackupName]
    wm title . "$progName: New System"
    popFlags
    doPushHistory 1
}

proc toggleImplicitSolver {} {
    global implicitSolver
    implicit $implicitSolver
}

# invokes OK or cancel button with given window, depending on current focus
proc invokeOKorCancel {window} {
    if [string equal [focus] "$window.cancel"] {
            $window.cancel invoke
        } else {
            $window.ok invoke
        }
}

set rkVars {
    timeUnit  "Time unit"
    stepMin   "Min Step Size"
    stepMax   "Max Step Size"
    nSteps     "No. steps per iteration"
    t0         "Start time"
    tmax      "Run until time"
    epsAbs     "Absolute error"
    epsRel     "Relative error"
    order      "Solver order (1,2 or 4)"
}

proc deiconifyRKDataForm {} {
    if {![winfo exists .rkDataForm]} {
        global rkVarInput rkVars
        toplevel .rkDataForm
        wm resizable .rkDataForm 0 0

        set row 0

        grid [label .rkDataForm.label$row -text "Simulation parameters"] -column 1 -columnspan 999 -pady 10
        incr row 10

        foreach {var text} $rkVars {
            set rowdict($text) $row
            grid [label .rkDataForm.label$row -text $text] -column 10 -row $row -sticky e
            grid [entry  .rkDataForm.text$row -width 20 -textvariable rkVarInput($var)] -column 20 -row $row -sticky ew
            incr row 10
        }
        grid [label .rkDataForm.implicitlabel -text "Implicit solver"] -column 10 -row $row -sticky e
        grid [checkbutton  .rkDataForm.implicitcheck -variable implicitSolver -command toggleImplicitSolver] -column 20 -row $row -sticky ew

        set rkVarInput(initial_focus) ".rkDataForm.text$rowdict(Min Step Size)"
        frame .rkDataForm.buttonBar
        button .rkDataForm.buttonBar.ok -text OK -command {setRKparms; closeRKDataForm}
        button .rkDataForm.buttonBar.cancel -text cancel -command {closeRKDataForm}
        pack .rkDataForm.buttonBar.ok [label .rkDataForm.buttonBar.spacer -width 2] .rkDataForm.buttonBar.cancel -side left -pady 10
        grid .rkDataForm.buttonBar -column 1 -row 999 -columnspan 999
        
        bind .rkDataForm <Key-Return> {invokeOKorCancel .rkDataForm.buttonBar}

        wm title .rkDataForm "Simulation parameters"
        # help bindings
        bind .rkDataForm  <F1>  {help RungeKutta}
        global helpTopics
        set helpTopics(.rkDataForm) RungeKutta
        bind .rkDataForm  <<contextMenu>> {helpContext %X %Y}
    } else {
        deiconify .rkDataForm
    }
}

proc closeRKDataForm {} {
    grab release .rkDataForm
    wm withdraw .rkDataForm
}

proc setRKparms {} {
    global rkVars rkVarInput
    foreach {var text} $rkVars { $var $rkVarInput($var) }
}


proc closePreferencesForm {} {
    destroy .preferencesForm
}

proc setPreferenceParms {} {
    global preferencesVars preferences preferences_input

    foreach var [array names preferences_input] {
	set preferences($var) $preferences_input($var)
    }
    defaultFont $preferences(defaultFont)
    multipleEquities $preferences(multipleEquities)
    setGodleyDisplay
    if {$preferences(focusFollowsMouse)} {
        tk_focusFollowsMouse
	# Make tab traversal possible within a window that is given focus by only clicking on it (no focusFollowsMouse). For ticket 901.
    } else {
       set old [bind all <Enter>]
       set script {
	   if {"%d" eq "NotifyAncestor" || "%d" eq "NotifyNonlinear" \
	   	|| "%d" eq "NotifyInferior"} {
	          tk::FocusOK %W	
	       }
       }
       if {$old ne ""} {
	   bind all <Enter> "$old; $script"
       } else {
	   bind all <Enter> $script
       }
    }
}

setPreferenceParms

# context sensitive help topics associations
set helpTopics(.#menubar) Menu
set helpTopics(.menubar.file) File
set helpTopics(.menubar.edit) Edit
set helpTopics(.menubar.ops) Insert
set helpTopics(.menubar.options) Options
set helpTopics(.controls.rec) RecReplayButtons
set helpTopics(.controls.runmode) RecReplayButtons
set helpTopics(.controls.run) RunButtons
set helpTopics(.controls.reset) RunButtons
set helpTopics(.controls.step) RunButtons
set helpTopics(.controls.simSpeed) Speedslider
set helpTopics(.controls.statusbar) SimTime
set helpTopics(.controls.zoomOut) ZoomButtons
set helpTopics(.controls.zoomIn) ZoomButtons
set helpTopics(.controls.zoomOrig)  ZoomButtons
set helpTopics(.controls.zoomFit)  ZoomButtons
# TODO - the following association interferes with canvas item context menus
# set helpTopics(.wiring.canvas) DesignCanvas


menu .contextHelp -tearoff 0
foreach win [array names helpTopics] {
    bind $win <<contextMenu>> {helpContext %X %Y}
}

# for binding to F1
proc topLevelHelp {} {
    helpFor [winfo pointerx .] [winfo pointery .]
}

# for binding to context menus
proc helpContext {x y} {
    .contextHelp delete 0 end
    .contextHelp add command -label Help -command "helpFor $x $y"
    tk_popup .contextHelp $x $y
}

# implements context sensistive help
proc helpFor {x y} {
    global helpTopics
    set win [winfo containing $x $y]
    if {$win==".wiring.canvas"} {
        canvasHelp
    } elseif [info exists helpTopics($win)] {
        help $helpTopics($win)
    } else {
        help Introduction
    }
}

proc canvasHelp {} {
    set x [get_pointer_x .wiring.canvas]
    set y [get_pointer_y .wiring.canvas]
    if [getItemAt $x $y] {
        help [minsky.canvas.item.classType]
    } elseif [getWireAt $x $y] {
        help wire
    } else {help DesignCanvas}
}

proc openURL {URL} {
    global tcl_platform
    if {[tk windowingsystem]=="win32"} {
        shellOpen $URL
    } elseif {$tcl_platform(os)=="Darwin"} {
        exec open $URL
    } elseif [catch {exec xdg-open $URL &}] {
        # try a few likely suspects
        foreach browser {firefox konqueror seamonkey opera} {
            set browserNotFound [catch {exec $browser $URL &}]
            if {!$browserNotFound} break
        }
        if $browserNotFound {
            tk_messageBox -detail "Unable to find a working web browser, 
please consult $URL" -type ok -icon warning
        }
    }
}

proc help {topic} {
    global minskyHome externalLabel
    # replace "Introduction" to framed toplevel document
    # TODO - see if it is possible to wrap the deep links with a framed service
    if {$topic=="Introduction"} {
        set URL  "file://$minskyHome/library/help/minsky.html"
    } else {
        set URL  "file://$minskyHome/library/help/minsky$externalLabel($topic)"
    }
    openURL $URL
}

proc aboutMinsky {} {
  tk_messageBox -message "
   Minsky [minskyVersion]\n
   Version used to save file [fileVersion]\n
   Tcl/Tk [info tclversion]\n
   Ravel [ravelVersion]
" -detail "
   Minsky is FREE software, distributed under the 
   GNU General Public License. It comes with ABSOLUTELY NO WARRANTY. 
   See http://www.gnu.org/licenses/ for details

   Some icons from the Antü Plasma Suita are licensed under Creative
   Commons Attribution-Share Alike 3.0 Unported license
   (https://creativecommons.org/licenses/by-sa/3.0/deed.en).

   Ravel is copyright Ravelation Pty Ltd. A separate license needs to
   be purchased to use Ravel. See https://ravelation.hpcoders.com.au

Thanks to following Minsky Unicorn sponsors:
     Edward McDaniel
     Travis Kimmel
   " 
}

# delete subsidiary toplevel such as godleys and plots
proc deleteSubsidiaryTopLevels {} {
    global globals

    canvas.defaultRotation 0
    set globals(godley_tables) {}

    foreach w [info commands .godley*] {destroy $w}
    foreach w [info commands .plot*] {destroy $w}
    foreach image [image names] {
        if [regexp ".plot.*|godleyImage.*|groupImage.*|varImage.*|opImage.*|plot_image.*" $image] {
                image delete $image
            }
    }
}

proc exit {} {
    # check if the model has been saved yet
    if {[edited]} {
        switch [tk_messageBox -message "Save before exiting?" -type yesnocancel] {
            yes save
            no {file delete -- [autoBackupName]}
            cancel {return -level [info level]}
        }
    }


    # if we have a valid rc file location, write out the directory of
    # the last file loaded
    global rcfile workDir backgroundColour preferences recentFiles
    if {$rcfile!=""} {
        set rc [open $rcfile w]
        puts $rc "set workDir $workDir"
        puts $rc "set canvasWidth [winfo width .wiring.canvas]"
        puts $rc "set canvasHeight [winfo height .wiring.canvas]"
        puts $rc "set backgroundColour $backgroundColour"
        foreach p [array names preferences] {
            puts $rc "set preferences($p) \{$preferences($p)\}"
        }
        puts $rc {minsky.defaultFont $preferences(defaultFont)}
        puts $rc "set recentFiles \{$recentFiles\}"
        close $rc
    }
    # why is this needed?
    proc bgerror x {} 
    tcl_exit
}

proc setFname {name} {
    global fname workDir recentFiles preferences progName
    if [string length $name] {
        set fname $name
        set workDir [file dirname $name]
        if {[lsearch $recentFiles $fname]==-1} {
            #add to recent files list
            if {[llength $recentFiles]>=$preferences(nRecentFiles)} {
                set recentFiles [lreplace $recentFiles 0 0]
            }
            lappend recentFiles $fname
            populateRecentFiles
        }
        # this strange piece of merde proved the only way I could test
        # for the presence of a close brace
        if {[regexp "\}$" $fname] && ![regexp "\}$" $workDir]} {
            set workDir "$workDir\}"
        }
        catch {wm title . "$progName: $fname"}
    }
}

# commands to redirect to wiringGroup (via unknown mechanism)
set getters {wire op constant integral data var plot godley group switchItem item items wires}
foreach i $getters {
    foreach j [info commands $i.*] {rename $j {}}
}
    
rename unknown ecolab_unknown
proc unknown {procname args} {
    #delegate in case a getter hasn't correctly called its get
    global getters
    if [regexp ^wiringGroup\. $procname] {
        # delegate to minsky (ie global group)
        eval [regsub ^wiringGroup $procname minsky] $args
    } elseif [regexp ^([join $getters |])\. $procname] {
        eval wiringGroup.$procname $args
    } else {
        eval ecolab_unknown $procname $args
    }
}

pushFlags

if {$argc>1} {
    #if argv(1) has .mdl extension, it is a Vensim model file
    if [string match "*.mdl" $argv(1)] {
        catch {eval importVensim $argv(1)}
        minsky.model.autoLayout
#        minsky.canvas.requestRedraw
        .controls.zoomFit invoke
    } elseif {![string match "*.tcl" $argv(1)]} {
        catch {eval openNamedFile {$argv(1)}}
    }
}

proc ifDef {var} {
    upvar $var v
    if {$v!="??"} {
        return "-$var $v "
    } else {
        return ""
    }
}

proc addEvent {event window button height state width x y delta keysym subwindow} {
    global eventRecord eventRecording
    if {$eventRecording && [info exists eventRecord] && [llength [file channels $eventRecord]]} {
        set rec "event generate $window $event "
        foreach option {button height state width x y delta keysym} {
            # for some reason, the logic misses this one
            if {($event=="<Key>" || $event=="<KeyRelease>") && $option=="delta"} continue
            append rec [ifDef $option]
        }
        puts $eventRecord $rec
    }
}

proc startRecording {filename} {
    if {[string length $filename]>0} {
        minsky.startRecording $filename
        recentreCanvas
    }
}

proc stopRecording {} {
    minsky.stopRecording
}

proc toggleRecording {} {
    global eventRecording workDir
    if $eventRecording {
        startRecording [tk_getSaveFile -filetypes {{"TCL scripts" .tcl TEXT} {"All Files" * }}\
                            -defaultextension .tcl -initialdir $workDir]
    } else {
        stopRecording
    }
}

proc checkRecordingVersion ver {
    if {$ver!=[minskyVersion]} {
        tk_messageBox -icon warning -message "Recording version $ver differs from current Minsky version [minskyVersion]" -detail "Recording may not replay correctly"
    }
}

# flag indicating we're in recording replay mode
set recordingReplay 0

proc replay {} {
    global recordingReplay eventRecordR workDir eventRecording
    if $eventRecording {stopRecording; set eventRecording 0}
    if {$recordingReplay} {
        # ensures consistent IDs are allocated
        set fname [tk_getOpenFile -filetypes {{"TCL scripts" .tcl TEXT} {"All Files" * }} \
                       -defaultextension .tcl -initialdir $workDir]
        if {[string length $fname]>0} {
            set eventRecordR [eval open {$fname} r]
            newSystem
            if {![running]} runstop
        } else {
            if [running] {runstop}
            set recordingReplay 0
        }
    } 
}

# check whether coverage analysis is required
if [info exists env(MINSKY_COV)] {attachTraceProc ::}

# a hook to allow code to be run after Minsky has initialised itself
if {[llength [info commands afterMinskyStarted]]>0} {
    afterMinskyStarted
}

setGodleyDisplayValue $preferences(godleyDisplay) $preferences(godleyDisplayStyle)
disableEventProcessing
popFlags
#pushHistory

