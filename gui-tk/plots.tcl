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

.menubar.ops add command -label Plot -command "addPlot"

proc deiconifyPltWindowOptions {} {
    if {![winfo exists .pltWindowOptions]} {
        global plotWindowOptions_legend plotWindowOptions_grid
        toplevel .pltWindowOptions
        frame .pltWindowOptions.xticks
        label .pltWindowOptions.xticks.label -text "Number of x ticks"
        entry  .pltWindowOptions.xticks.val -width 20
        pack .pltWindowOptions.xticks.label .pltWindowOptions.xticks.val  -side left

        frame .pltWindowOptions.yticks
        label .pltWindowOptions.yticks.label -text "Number of y ticks"
        entry  .pltWindowOptions.yticks.val -width 20
        pack .pltWindowOptions.yticks.label .pltWindowOptions.yticks.val  -side left

        frame .pltWindowOptions.xtickAngle
        label .pltWindowOptions.xtickAngle.label -text "x ticks angle"
        entry  .pltWindowOptions.xtickAngle.val -width 20
        pack .pltWindowOptions.xtickAngle.label .pltWindowOptions.xtickAngle.val  -side left

        frame .pltWindowOptions.exp_threshold
        label .pltWindowOptions.exp_threshold.label -text "exp threshold"
        entry  .pltWindowOptions.exp_threshold.val -width 20
        pack .pltWindowOptions.exp_threshold.label .pltWindowOptions.exp_threshold.val  -side left

        frame .pltWindowOptions.plotType
        label .pltWindowOptions.plotType.label -text "Plot type"
        ttk::combobox  .pltWindowOptions.plotType.val -width 20 -state readonly -textvariable plotWindowOptions(plotType) -value {line bar automatic}
        pack .pltWindowOptions.plotType.label .pltWindowOptions.plotType.val  -side left
        
        
        
        frame .pltWindowOptions.options
        checkbutton .pltWindowOptions.options.grid -text "Grid" -variable plotWindowOptions(grid)
        checkbutton .pltWindowOptions.options.subgrid -text "Subgrid" -variable plotWindowOptions(subgrid)
        checkbutton .pltWindowOptions.options.legend -text "Legend:" -variable plotWindowOptions(legend)
        pack .pltWindowOptions.options.grid .pltWindowOptions.options.subgrid .pltWindowOptions.options.legend -side left
        
        frame .pltWindowOptions.logscale
        checkbutton .pltWindowOptions.logscale.xv -text "x log scale" -variable plotWindowOptions(xlog)
        checkbutton .pltWindowOptions.logscale.yv -text "y log scale" -variable plotWindowOptions(ylog)
        pack .pltWindowOptions.logscale.xv  .pltWindowOptions.logscale.yv  -side left

        frame .pltWindowOptions.title
        label .pltWindowOptions.title.label -text Title
        entry .pltWindowOptions.title.val -width 20
        pack .pltWindowOptions.title.label .pltWindowOptions.title.val -side left

        frame .pltWindowOptions.xaxislabel
        label .pltWindowOptions.xaxislabel.label -text "X label"
        entry .pltWindowOptions.xaxislabel.val -width 20
        pack .pltWindowOptions.xaxislabel.label .pltWindowOptions.xaxislabel.val -side left
        
        frame .pltWindowOptions.yaxislabel
        label .pltWindowOptions.yaxislabel.label -text "Y label"
        entry .pltWindowOptions.yaxislabel.val -width 20
        pack .pltWindowOptions.yaxislabel.label .pltWindowOptions.yaxislabel.val -side left
        
        frame .pltWindowOptions.y1axislabel
        label .pltWindowOptions.y1axislabel.label -text "RHS Y label"
        entry .pltWindowOptions.y1axislabel.val -width 20
        pack .pltWindowOptions.y1axislabel.label .pltWindowOptions.y1axislabel.val -side left

        pack .pltWindowOptions.title .pltWindowOptions.xaxislabel .pltWindowOptions.yaxislabel .pltWindowOptions.y1axislabel .pltWindowOptions.plotType

        frame .pltWindowOptions.buttonBar
        button .pltWindowOptions.buttonBar.ok -text OK
        button .pltWindowOptions.buttonBar.cancel -text Cancel -command {
            wm withdraw .pltWindowOptions
            grab release .pltWindowOptions 
        }
        pack .pltWindowOptions.buttonBar.ok .pltWindowOptions.buttonBar.cancel -side left
        pack .pltWindowOptions.buttonBar -side bottom

        pack .pltWindowOptions.xticks .pltWindowOptions.yticks .pltWindowOptions.options .pltWindowOptions.logscale
    } else {
        wm deiconify .pltWindowOptions
    }
}

deiconifyPltWindowOptions
wm withdraw .pltWindowOptions
        
proc setPlotOptions {plot} {
    global plotWindowOptions
    $plot.grid $plotWindowOptions(grid)
    $plot.subgrid $plotWindowOptions(subgrid)
    $plot.logx $plotWindowOptions(xlog)
    $plot.logy $plotWindowOptions(ylog)
    $plot.percent $plotWindowOptions(ypercent)
    $plot.plotType $plotWindowOptions(plotType)
    $plot.nxTicks [.pltWindowOptions.xticks.val get]
    $plot.nyTicks [.pltWindowOptions.yticks.val get]
    $plot.xtickAngle [.pltWindowOptions.xtickAngle.val get]
    $plot.exp_threshold [.pltWindowOptions.exp_threshold.val get]
    $plot.title [.pltWindowOptions.title.val get]
    $plot.xlabel [.pltWindowOptions.xaxislabel.val get]
    $plot.ylabel [.pltWindowOptions.yaxislabel.val get]
    $plot.y1label [.pltWindowOptions.y1axislabel.val get]
    $plot.legend $plotWindowOptions(legend)
    $plot.legendSide boundingBox
    canvas.requestRedraw
    catch {wm title .plot$id [plot.title]}
    wm withdraw .pltWindowOptions 
    grab release .pltWindowOptions 
}

proc doPlotOptions {plot} {
    global plotWindowOptions
    set plotWindowOptions(grid) [$plot.grid]
    set plotWindowOptions(subgrid) [$plot.subgrid]
    set plotWindowOptions(xlog) [$plot.logx]
    set plotWindowOptions(ylog) [$plot.logy]
    set plotWindowOptions(ypercent) [$plot.percent]
    set plotWindowOptions(plotType) [$plot.plotType]
    deiconifyPltWindowOptions

    .pltWindowOptions.xticks.val delete 0 end
    .pltWindowOptions.xticks.val insert 0 [$plot.nxTicks]
    .pltWindowOptions.yticks.val delete 0 end
    .pltWindowOptions.yticks.val insert 0 [$plot.nyTicks]
    .pltWindowOptions.xtickAngle.val delete 0 end
    .pltWindowOptions.xtickAngle.val insert 0 [$plot.xtickAngle]
    .pltWindowOptions.exp_threshold.val delete 0 end
    .pltWindowOptions.exp_threshold.val insert 0 [$plot.exp_threshold]
    .pltWindowOptions.title.val delete 0 end
    .pltWindowOptions.title.val insert 0 [$plot.title]
    .pltWindowOptions.xaxislabel.val delete 0 end
    .pltWindowOptions.xaxislabel.val insert 0 [$plot.xlabel]
    .pltWindowOptions.yaxislabel.val delete 0 end
    .pltWindowOptions.yaxislabel.val insert 0 [$plot.ylabel]
    .pltWindowOptions.y1axislabel.val delete 0 end
    .pltWindowOptions.y1axislabel.val insert 0 [$plot.y1label]

    .pltWindowOptions.buttonBar.ok configure -command "setPlotOptions $plot"
    set plotWindowOptions(legend) [$plot.legend]
    grab .pltWindowOptions
}

# double click handling for plot (creates new toplevel plot window)
proc plotDoubleClick {plotId} {
    toplevel .plot$plotId
    wm title .plot$plotId [$plotId.title]
    
    #pack [canvas .plots.canvas$plotId] -fill both -expand 1

    labelframe .plot$plotId.menubar -relief raised
    button .plot$plotId.menubar.options -text Options -command "doPlotOptions $plotId" -relief flat
    pack .plot$plotId.menubar.options -side left

    pack .plot$plotId.menubar  -side top -fill x

    image create cairoSurface .plot$plotId.image -surface $plotId -width 400 -height 400
    $plotId.deleteCallback "destroy .plot$plotId"
    label .plot$plotId.label -image .plot$plotId.image -width 400 -height 400
    pack .plot$plotId.label -fill both -expand 1
}

proc hex x {
    return [format "%02x" [expr int(256*($x-1e-3))]]
}
    
# convert an ecolab::cairo::Colour object to a Tk_Colour name
proc colourName {x} {
    return "#[hex [$x.r]][hex [$x.g]][hex [$x.b]]"
}

proc configColourButton b {
    $b configure -background [tk_chooseColor -initialcolor [$b cget -background]]
}

proc makeRow {i p} {
    frame .penStyles.row$i
    label .penStyles.row$i.no -text $i
    button .penStyles.row$i.colour -background [colourName $p.colour] -command "configColourButton .penStyles.row$i.colour"
    entry .penStyles.row$i.width -width 10
    .penStyles.row$i.width insert 0 [$p.width]
    ttk::combobox .penStyles.row$i.style -width 10 -state readonly -values {"————" "- - -" "· · ·" "- · -"}
    pack .penStyles.row$i.no .penStyles.row$i.colour .penStyles.row$i.width .penStyles.row$i.style -side left
}

proc penStyles {plot} {
    toplevel .penStyles
    for {set i 0} {$i<[$plot.palette.size]} {incr i} {
        makeRow $i [$plot.palette.@elem $i]
        pack .penStyles.row$i
    }
    buttonBar .penStyles "penStyleOK $plot"
    button .penStyles.buttonBar.add -text "+" -command "addRow $plot"
    pack .penStyles.buttonBar.add -before .penStyles.buttonBar.cancel -side left
    grab set .penStyles
    wm transient .penStyles
}

proc penStyleOK plot {
    for {set i 0} {$i<[$plot.palette.size]} {incr i} {
        set p [$plot.palette.@elem $i]
        minsky.setColour $i  [.penStyles.row$i.colour cget -background]
        $p.width [.penStyles.row$i.width get]
        switch [.penStyles.row$i.style get] {
            "————" {$p.dashStyle solid}
            "- - -" {$p.dashStyle dash}
            "· · ·" {$p.dashStyle dot}
            "- · -" {$p.dashStyle dashDot}
        }
    }
    $plot.requestRedraw
}

proc addRow plot {
    set i [$plot.palette.size]
    $plot.extendPalette
    makeRow $i [$plot.palette.@elem $i]
    pack .penStyles.row$i -before .penStyles.buttonBar
}
