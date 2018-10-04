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

        frame .pltWindowOptions.plotType
        label .pltWindowOptions.plotType.label -text "Plot type"
        ttk::combobox  .pltWindowOptions.plotType.val -width 20 -state readonly -textvariable plotWindowOptions(plotType) -value {line bar}
        pack .pltWindowOptions.plotType.label .pltWindowOptions.plotType.val  -side left
        
        
        
        frame .pltWindowOptions.grid
        label .pltWindowOptions.grid.label -text "Grid"
        label .pltWindowOptions.grid.sublabel -text "Subgrid"
        checkbutton .pltWindowOptions.grid.val -variable plotWindowOptions(grid)
        checkbutton .pltWindowOptions.grid.subval -variable plotWindowOptions(subgrid)

        frame .pltWindowOptions.logscale
        label .pltWindowOptions.logscale.x -text "x log scale"
        label .pltWindowOptions.logscale.y -text "y log scale"
        checkbutton .pltWindowOptions.logscale.xv -variable plotWindowOptions(xlog)
        checkbutton .pltWindowOptions.logscale.yv -variable plotWindowOptions(ylog)

        frame .pltWindowOptions.legend
        label .pltWindowOptions.legend.label -text "Legend:"
        label .pltWindowOptions.legend.noneLabel -text none
        radiobutton .pltWindowOptions.legend.none -variable plotWindowOptions(legend) -value none
        label .pltWindowOptions.legend.leftLabel -text left
        radiobutton .pltWindowOptions.legend.left -variable plotWindowOptions(legend) -value left
        label .pltWindowOptions.legend.rightLabel -text right
        radiobutton .pltWindowOptions.legend.right -variable plotWindowOptions(legend) -value right
        pack .pltWindowOptions.legend.label .pltWindowOptions.legend.none .pltWindowOptions.legend.noneLabel .pltWindowOptions.legend.left .pltWindowOptions.legend.leftLabel .pltWindowOptions.legend.right .pltWindowOptions.legend.rightLabel -side left

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

        pack .pltWindowOptions.grid.label  .pltWindowOptions.grid.val  .pltWindowOptions.grid.sublabel  .pltWindowOptions.grid.subval  -side left

        pack .pltWindowOptions.logscale.x  .pltWindowOptions.logscale.xv  .pltWindowOptions.logscale.y  .pltWindowOptions.logscale.yv  -side left

        frame .pltWindowOptions.buttonBar
        button .pltWindowOptions.buttonBar.ok -text OK
        button .pltWindowOptions.buttonBar.cancel -text Cancel -command {
            wm withdraw .pltWindowOptions
            grab release .pltWindowOptions 
        }
        pack .pltWindowOptions.buttonBar.ok .pltWindowOptions.buttonBar.cancel -side left
        pack .pltWindowOptions.buttonBar -side bottom

        pack .pltWindowOptions.xticks .pltWindowOptions.yticks .pltWindowOptions.xtickAngle .pltWindowOptions.grid .pltWindowOptions.legend .pltWindowOptions.logscale
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
    $plot.plotType $plotWindowOptions(plotType)
    $plot.nxTicks [.pltWindowOptions.xticks.val get]
    $plot.nyTicks [.pltWindowOptions.yticks.val get]
    $plot.xtickAngle [.pltWindowOptions.xtickAngle.val get]
    $plot.title [.pltWindowOptions.title.val get]
    $plot.xlabel [.pltWindowOptions.xaxislabel.val get]
    $plot.ylabel [.pltWindowOptions.yaxislabel.val get]
    $plot.y1label [.pltWindowOptions.y1axislabel.val get]
    if {$plotWindowOptions(legend)=="none"} {
        $plot.legend 0
    } else {
        $plot.legend 1
        $plot.legendSide $plotWindowOptions(legend)
    }
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
    set plotWindowOptions(plotType) [$plot.plotType]
    deiconifyPltWindowOptions

    .pltWindowOptions.xticks.val delete 0 end
    .pltWindowOptions.xticks.val insert 0 [$plot.nxTicks]
    .pltWindowOptions.yticks.val delete 0 end
    .pltWindowOptions.yticks.val insert 0 [$plot.nyTicks]
    .pltWindowOptions.xtickAngle.val delete 0 end
    .pltWindowOptions.xtickAngle.val insert 0 [$plot.xtickAngle]
    .pltWindowOptions.title.val delete 0 end
    .pltWindowOptions.title.val insert 0 [$plot.title]
    .pltWindowOptions.xaxislabel.val delete 0 end
    .pltWindowOptions.xaxislabel.val insert 0 [$plot.xlabel]
    .pltWindowOptions.yaxislabel.val delete 0 end
    .pltWindowOptions.yaxislabel.val insert 0 [$plot.ylabel]
    .pltWindowOptions.y1axislabel.val delete 0 end
    .pltWindowOptions.y1axislabel.val insert 0 [$plot.y1label]

    .pltWindowOptions.buttonBar.ok configure -command "setPlotOptions $plot"
    global plotWindowOptions_legend
    if [$plot.legend] {
        switch [$plot.legendSide] {
            0 {set plotWindowOptions(legend) left}
            1 {set plotWindowOptions(legend) right}
        }
    } else {set plotWindowOptions(legend) none}
        
    grab .pltWindowOptions
}

# double click handling for plot (creates new toplevel plot window)
proc plotDoubleClick {plotId} {
    toplevel .plot$plotId
    wm title .plot$plotId [$plotId.title]

    labelframe .plot$plotId.menubar -relief raised
    button .plot$plotId.menubar.options -text Options -command "doPlotOptions $plotId" -relief flat
    pack .plot$plotId.menubar.options -side left

    pack .plot$plotId.menubar  -side top -fill x

    image create cairoSurface .plot$plotId.image -surface $plotId -width 400 -height 400
    label .plot$plotId.label -image .plot$plotId.image -width 400 -height 400
    pack .plot$plotId.label -fill both -expand 1
}
    
