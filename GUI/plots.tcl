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

.menubar.ops add command -label Plot -command "newPlot"

proc newPlotItem {id x y} {
    plot.get $id
    .wiring.canvas create plot $x $y -id $id -tags "plots plot$id"
    .wiring.canvas lower plot$id
    setM1Binding plot $id plot$id
    .wiring.canvas bind plot$id <Double-Button-1> "plotDoubleClick $id"
    .wiring.canvas bind plot$id <Enter> "itemEnterLeave plot $id plot$id 1"
    .wiring.canvas bind plot$id <Leave> "itemEnterLeave plot $id plot$id 0"

}

proc newPlot {} {
    # place this at the mouse if in canvas, otherwise at 0 0

    set id [minsky.newPlot 0 0]
    newPlotItem $id 0 0
    plot.get $id
    plot.zoomFactor [zoomFactor]
    plot.set

    global moveOffsplot$id.x moveOffsplot$id.y
    set moveOffsplot$id.x 0
    set moveOffsplot$id.y 0

    bind .wiring.canvas <Enter> "move plot $id %x %y"
    bind .wiring.canvas <Motion> "move plot $id %x %y"
    bind .wiring.canvas <Button-1> clearTempBindings
    bind . <Key-Escape> "clearTempBindings; deletePlot plot$id $id"

    return $id
}

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
        
        frame .pltWindowOptions.grid
        label .pltWindowOptions.grid.label -text "Grid"
        label .pltWindowOptions.grid.sublabel -text "Subgrid"
        checkbutton .pltWindowOptions.grid.val -variable plotWindowOptions_grid -command {plot.grid $plotWindowOptions_grid}
        checkbutton .pltWindowOptions.grid.subval -variable plotWindowOptions_subgrid -command {plot.subgrid $plotWindowOptions_subgrid}

        frame .pltWindowOptions.legend
        label .pltWindowOptions.legend.label -text "Legend:"
        label .pltWindowOptions.legend.noneLabel -text none
        radiobutton .pltWindowOptions.legend.none -variable plotWindowOptions_legend -command plot::setLegend -value none
        label .pltWindowOptions.legend.leftLabel -text left
        radiobutton .pltWindowOptions.legend.left -variable plotWindowOptions_legend -command plot::setLegend -value left
        label .pltWindowOptions.legend.rightLabel -text right
        radiobutton .pltWindowOptions.legend.right -variable plotWindowOptions_legend -command plot::setLegend -value right
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

        pack .pltWindowOptions.title .pltWindowOptions.xaxislabel .pltWindowOptions.yaxislabel .pltWindowOptions.y1axislabel

        pack .pltWindowOptions.grid.label  .pltWindowOptions.grid.val  .pltWindowOptions.grid.sublabel  .pltWindowOptions.grid.subval  -side left

        frame .pltWindowOptions.buttonBar
        button .pltWindowOptions.buttonBar.ok -text OK
        button .pltWindowOptions.buttonBar.cancel -text Cancel -command {
            wm withdraw .pltWindowOptions
            grab release .pltWindowOptions 
        }
        pack .pltWindowOptions.buttonBar.ok .pltWindowOptions.buttonBar.cancel -side left
        pack .pltWindowOptions.buttonBar -side bottom

        pack .pltWindowOptions.xticks .pltWindowOptions.yticks .pltWindowOptions.grid .pltWindowOptions.legend
    } else {
        wm deiconify .pltWindowOptions
    }
}

set plotWindowOptions_legend none
deiconifyPltWindowOptions
wm withdraw .pltWindowOptions
set plotWindowOptions_grid
set plotWindowOptions_subgrid
        
proc setPlotOptions {id} {
    global plotWindowOptions_grid plotWindowOptions_subgrid
# TODO this can lose data if sim is running. When plot becomes an actual reference, not a copy, this problem will be averted
    plot.get $id
    plot.grid $plotWindowOptions_grid
    plot.subgrid $plotWindowOptions_subgrid
    plot.nxTicks [.pltWindowOptions.xticks.val get]
    plot.nyTicks [.pltWindowOptions.yticks.val get]
    plot.title [.pltWindowOptions.title.val get]
    plot.xlabel [.pltWindowOptions.xaxislabel.val get]
    plot.ylabel [.pltWindowOptions.yaxislabel.val get]
    plot.y1label [.pltWindowOptions.y1axislabel.val get]
    plot.set
    plot.redraw
    catch {wm title .plot$id [plot.title]}
    wm withdraw .pltWindowOptions 
    grab release .pltWindowOptions 
}

proc doPlotOptions {id} {
    global plotWindowOptions_grid plotWindowOptions_subgrid
    plot.get $id
    set plotWindowOptions_grid [plot.grid]
    set plotWindowOptions_subgrid [plot.subgrid]

    deiconifyPltWindowOptions

    .pltWindowOptions.xticks.val delete 0 end
    .pltWindowOptions.xticks.val insert 0 [plot.nxTicks]
    .pltWindowOptions.yticks.val delete 0 end
    .pltWindowOptions.yticks.val insert 0 [plot.nyTicks]
    .pltWindowOptions.title.val delete 0 end
    .pltWindowOptions.title.val insert 0 [plot.title]
    .pltWindowOptions.xaxislabel.val delete 0 end
    .pltWindowOptions.xaxislabel.val insert 0 [plot.xlabel]
    .pltWindowOptions.yaxislabel.val delete 0 end
    .pltWindowOptions.yaxislabel.val insert 0 [plot.ylabel]
    .pltWindowOptions.y1axislabel.val delete 0 end
    .pltWindowOptions.y1axislabel.val insert 0 [plot.y1label]

    .pltWindowOptions.buttonBar.ok configure -command "setPlotOptions $id"
    global plotWindowOptions_legend
    if [plot.legend] {
        switch [plot.legendSide] {
            0 {set plotWindowOptions_legend left}
            1 {set plotWindowOptions_legend right}
        }
    } else {set plotWindowOptions_legend none}
        
    grab .pltWindowOptions
}

# w and h are requested window size, dw, dh are difference between
# image and window dimensions
proc resizePlot {id w h dw dh} {
    if {[winfo width .plot$id]!=[expr [.plot$id.image cget -width]+$dw] ||
        [winfo height .plot$id]!=[expr [.plot$id.image cget -height]+$dh]} {
        .plot$id.image configure -height [expr [winfo height .plot$id]-$dh] -width [expr [winfo width .plot$id]-$dw]
        plots.addImage $id .plot$id.image
    }
}

# double click handling for plot (creates new toplevel plot window)
proc plotDoubleClick {id} {

    toplevel .plot$id
    plot.get $id
    wm title .plot$id [plot.title]
    image create photo .plot$id.image -width 500 -height 500
    label .plot$id.label -image .plot$id.image

    labelframe .plot$id.menubar -relief raised
    button .plot$id.menubar.options -text Options -command "doPlotOptions $id" -relief flat
    pack .plot$id.menubar.options -side left

    pack .plot$id.menubar  -side top -fill x
    pack .plot$id.label
    
    plots.addImage $id .plot$id.image

    # calculate the difference between the window an image sizes
    update
    set dw [expr [winfo width .plot$id]-[.plot$id.image cget -width]]
    set dh [expr [winfo height .plot$id]-[.plot$id.image cget -height]]

    bind .plot$id <Configure> "resizePlot $id  %w %h $dw $dh"
}
    
proc deletePlot {item id} {
    .wiring.canvas delete $item
    minsky.deletePlot $id
    updateCanvas
}
    
namespace eval plot {
    proc resize {id} {
        plot.get $id
        set bbox [.wiring.canvas bbox plot$id]
        variable orig_width [expr [lindex $bbox 2]-[lindex $bbox 0]]
        variable orig_height [expr [lindex $bbox 3]-[lindex $bbox 1]]
        variable orig_x [plot.x]
        variable orig_y [plot.y]
        set item [eval .wiring.canvas create rectangle $bbox -tags plotBBox]
        # disable lasso mode
        bind .wiring.canvas <Button-1> ""
        bind .wiring.canvas <B1-Motion> ""
        bind .wiring.canvas <B1-ButtonRelease> ""
        bind .wiring.canvas <Motion> "plot::resizeRect $item %x %y"
        bind .wiring.canvas <ButtonRelease> "plot::resizeItem $item $id %x %y"
    }

    # resize the bounding box to indicate how big we want the icon to be
    proc resizeRect {item x y} {
        set x [.wiring.canvas canvasx $x]
        set y [.wiring.canvas canvasy $y]
        variable orig_x
        variable orig_y
        set w [expr abs($x-$orig_x)]
        set h [expr abs($y-$orig_y)]
        .wiring.canvas coords $item  [expr $orig_x-$w] [expr $orig_y-$h] \
            [expr $orig_x+$w] [expr $orig_y+$h]
    }

    # compute width and height and redraw item
    proc resizeItem {item id x y} {
        plot.get $id
        set x [.wiring.canvas canvasx $x]
        set y [.wiring.canvas canvasy $y]
        .wiring.canvas delete $item
        variable orig_width
        variable orig_height
        variable orig_x
        variable orig_y
        set scalex [expr 2*abs($x-$orig_x)/double($orig_width)]
        set scaley [expr 2*abs($y-$orig_y)/double($orig_height)]
        # compute rotated scale factors
        plot.width [expr int(ceil(abs($scalex*[plot.width])))]
        plot.height [expr int(ceil(abs($scaley*[plot.height])))]

        .wiring.canvas delete plot$id
        newPlotItem $id [plot.x] [plot.y]
        foreach p [plot.ports]  {
            adjustWire $p
        }
        bind .wiring.canvas <Motion> {}
        bind .wiring.canvas <ButtonRelease> {}
        setInteractionMode
    }

    proc setLegend {} {
        global plotWindowOptions_legend
        switch $plotWindowOptions_legend {
            none {
                plot.legend 0
            }
            left {
                plot.legend 1
                plot.legendSide left
            }
            right {
                plot.legend 1
                plot.legendSide right
            }
        }
    }
}
