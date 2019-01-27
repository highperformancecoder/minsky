# assume canvas.minsky.item is the variable
CSVDialog csvDialog
proc CSVImportDialog {} {
    global workDir csvParms
    if {![winfo exists .wiring.csvImport]} {
        toplevel .wiring.csvImport
        frame .wiring.csvImport.delimiters
        label .wiring.csvImport.delimiters.columnarLabel -text "Columnar"
        ttk::checkbutton .wiring.csvImport.delimiters.columnar -variable csvParms(columnar) -command {
            csvDialog.spec.columnar $csvParms(columnar)
            csvDialog.requestRedraw
        }
        label .wiring.csvImport.delimiters.separatorLabel -text Separator
        ttk::combobox .wiring.csvImport.delimiters.separatorValue -values {
            "," ";" "<tab>" "<space>"} -textvariable csvParms(separator) -width 5
        bind .wiring.csvImport.delimiters.separatorValue <<ComboboxSelected>> {
            csvDialog.spec.separator $csvParms(separator)}
        label .wiring.csvImport.delimiters.escapeLabel -text Escape
        ttk::combobox .wiring.csvImport.delimiters.escapeValue -values {
            "\\"} -textvariable csvParms(escape) -width 5
        bind .wiring.csvImport.delimiters.escapeValue <<ComboboxSelected>> {
            csvDialog.spec.escape $csvParms(escape)}
        label .wiring.csvImport.delimiters.quoteLabel -text Quote
        ttk::combobox .wiring.csvImport.delimiters.quoteValue -values {
            "'" "\"" } -textvariable csvParms(quote) -width 5
        bind .wiring.csvImport.delimiters.quoteValue <<ComboboxSelected>> {
            csvDialog.spec.quote $csvParms(quote)}
        label .wiring.csvImport.delimiters.mergeLabel -text "Merge Delimiters"
        ttk::checkbutton .wiring.csvImport.delimiters.mergeValue -variable csvParms(mergeDelimiters) -command {
            csvDialog.spec.mergeDelimiters $csvParms(mergeDelimiters)
        }
        label .wiring.csvImport.delimiters.missingLabel -text "Missing Value"
        ttk::combobox .wiring.csvImport.delimiters.missingValue \
            -textvariable csvParms(missingValue) -values {"nan" "0"} -width 5
        bind .wiring.csvImport.delimiters.missingValue <<ComboboxSelected>> {
            csvDialog.spec.missingValue $csvParms(missingValue)}

        pack .wiring.csvImport.delimiters.columnarLabel .wiring.csvImport.delimiters.columnar \
            .wiring.csvImport.delimiters.separatorLabel .wiring.csvImport.delimiters.separatorValue \
            .wiring.csvImport.delimiters.escapeLabel .wiring.csvImport.delimiters.escapeValue \
            .wiring.csvImport.delimiters.quoteLabel .wiring.csvImport.delimiters.quoteValue \
            .wiring.csvImport.delimiters.mergeLabel .wiring.csvImport.delimiters.mergeValue \
            .wiring.csvImport.delimiters.missingLabel .wiring.csvImport.delimiters.missingValue -side left

        pack .wiring.csvImport.delimiters
        
        frame .wiring.csvImport.horizontalName
        label .wiring.csvImport.horizontalName.text -text "Horizontal dimension"
        entry .wiring.csvImport.horizontalName.value -textvariable csvParms(horizontalDimension)
        pack .wiring.csvImport.horizontalName.text .wiring.csvImport.horizontalName.value -side left
        pack .wiring.csvImport.horizontalName

        image create cairoSurface csvDialogTable -surface csvDialog
        label .wiring.csvImport.table -image csvDialogTable -width 800 -height 300
        pack .wiring.csvImport.table -fill both -expand 1 -side top

        buttonBar .wiring.csvImport {
            csvDialog.spec.horizontalDimName $csvParms(horizontalDimension)
            loadVariableFromCSV csvDialog.spec $csvParms(filename)
            reset
        }
        bind .wiring.csvImport.table <Configure> "csvDialog.requestRedraw"
        bind .wiring.csvImport.table <Button-1> {csvImportButton1 %x %y};
        bind .wiring.csvImport.table <ButtonRelease-1> {csvImportButton1Up %x %y %X %Y};
        bind .wiring.csvImport.table <B1-Motion> {csvDialog.xoffs [expr $csvImportPanX+%x]; csvDialog.requestRedraw}
    }
    set filename [tk_getOpenFile -filetypes {{CSV {.csv}} {All {.*}}} -initialdir $workDir]
    if [string length $filename] {
        csvDialog.loadFile $filename
        set csvParms(filename) $filename
        set csvParms(separator) [csvDialog.spec.separator]
        set csvParms(escape) [csvDialog.spec.escape]
        set csvParms(quote) [csvDialog.spec.quote]
        set csvParms(mergeDelimiters) [csvDialog.spec.mergeDelimiters]
        set csvParms(missingValue) [csvDialog.spec.missingValue]
        set csvParms(horizontalDimension) [csvDialog.spec.horizontalDimName]
        wm deiconify .wiring.csvImport
        raise .wiring.csvImport
        csvDialog.requestRedraw
    }
}

proc csvImportButton1 {x y} {
    global csvImportPanX mouseSave
    set csvImportPanX [expr [csvDialog.xoffs]-$x]
    set mouseSave "$x $y"
}

proc closeCombo setter {
    puts $setter
    eval $setter \[.wiring.csvImport.text.combo get\]
    wm withdraw .wiring.csvImport.text
    csvDialog.requestRedraw
}

proc setupCombo {getter setter title configure X Y} {
    wm title .wiring.csvImport.text $title
    eval .wiring.csvImport.text.combo configure $configure
    .wiring.csvImport.text.combo set $getter
    bind .wiring.csvImport.text.combo <<ComboboxSelected>> "closeCombo $setter"
    bind .wiring.csvImport.text.combo <Return> "closeCombo $setter"
    wm deiconify .wiring.csvImport.text
    wm geometry .wiring.csvImport.text +$X+$Y
    raise .wiring.csvImport.text 
}

proc csvImportButton1Up {x y X Y} {
    global mouseSave csvParms
    # combobox used for setting dimension type
    if {![winfo exists .wiring.csvImport.text.combo]} {
        toplevel .wiring.csvImport.text
        ttk::combobox .wiring.csvImport.text.combo -values {"string" "value" "time"} -state readonly
        pack .wiring.csvImport.text.combo
        wm withdraw .wiring.csvImport.text
    }

    set col [csvDialog.columnOver $x]
    set row [csvDialog.rowOver $y]
    if {abs([lindex $mouseSave 0]-$x)==0 && abs([lindex $mouseSave 1]-$y)==0} {
        # mouse click - implement dialog interaction logic
        switch [csvDialog.rowOver $y] {
            0 {csvDialog.spec.toggleDimension $col
                csvDialog.requestRedraw
            }
            1 {if {$col<[csvDialog.spec.dimensions.size]} {
                setupCombo [[csvDialog.spec.dimensions.@elem $col].type] \
                    "csvDialog.spec.dimensions($col).type" \
                    "Dimension type" {-values {"string" "value" "time"} -state readonly} $X $Y
            }}
            2 {if {$col<[csvDialog.spec.dimensions.size]} {
                set units ""
                if {[[csvDialog.spec.dimensions.@elem $col].type]=="time"} {
                    set units "{%Y-%m-%D %Y-%m-%d %H:%M:%S %Y-Q%Q}"
                }
                setupCombo [[csvDialog.spec.dimensions.@elem $col].units] \
                    "csvDialog.spec.dimensions($col).units" \
                    "Dimension units/format" "-values $units -state normal" $X $Y
            }}
            3 {if {$col<[csvDialog.spec.dimensions.size]} {
                setupCombo [[csvDialog.spec.dimensionNames.@elem $col]] \
                    "csvDialog.spec.dimensionNames($col)" \
                    "Dimension name" {-values {} -state normal} $X $Y
            }}
            default {
                csvDialog.spec.nRowAxes [expr $row-4]
                csvDialog.spec.nColAxes $col
                csvDialog.requestRedraw
            }
        }
    }
}
