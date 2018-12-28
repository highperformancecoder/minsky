# assume canvas.minsky.item is the variable
CSVDialog csvDialog
proc CSVImportDialog {} {
    global workDir csvParms
    if {![winfo exists .wiring.csvImport]} {
        toplevel .wiring.csvImport
        frame .wiring.csvImport.delimiters
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

        pack .wiring.csvImport.delimiters.separatorLabel .wiring.csvImport.delimiters.separatorValue \
            .wiring.csvImport.delimiters.escapeLabel .wiring.csvImport.delimiters.escapeValue \
            .wiring.csvImport.delimiters.quoteLabel .wiring.csvImport.delimiters.quoteValue \
            .wiring.csvImport.delimiters.mergeLabel .wiring.csvImport.delimiters.mergeValue \
            .wiring.csvImport.delimiters.missingLabel .wiring.csvImport.delimiters.missingValue -side left

        pack .wiring.csvImport.delimiters
        
        image create cairoSurface csvDialogTable -surface csvDialog
        label .wiring.csvImport.table -image csvDialogTable -width 800 -height 300
        pack .wiring.csvImport.table -fill both -expand 1
        bind .wiring.csvImport.table <<contextMenu>> {csvImportMenu %x %y %X %Y}

        buttonBar .wiring.csvImport {loadVariableFromCSV csvDialog.spec $csvParms(filename)}
        menu .wiring.csvImport.context
        bind .wiring.csvImport.table <Configure> "csvDialog.requestRedraw"
        bind .wiring.csvImport.table <Button-1> {set csvImportPanX [expr [csvDialog.xoffs]-%x]};
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
        wm deiconify .wiring.csvImport
        raise .wiring.csvImport
        csvDialog.requestRedraw
    }
}

proc csvImportMenu {x y X Y} {
    .wiring.csvImport.context delete 0 end
    set col [csvDialog.columnOver $x]
    set row [csvDialog.rowOver $y]
    .wiring.csvImport.context add command -label "Data start row/col" -command "
        csvDialog.spec.nRowAxes $row
        csvDialog.spec.nColAxes $col"
    .wiring.csvImport.context add separator
    .wiring.csvImport.context add command -label "Comment row" -command "csvDialog.spec.commentRow $row"
    .wiring.csvImport.context add command -label "Uncomment row" -command "csvDialog.spec.uncommentRow $row"
    .wiring.csvImport.context add separator
    .wiring.csvImport.context add command -label "Comment column" -command "csvDialog.spec.commentCol $col"
    .wiring.csvImport.context add command -label "Uncomment column" -command "csvDialog.spec.uncommentCol $col"
    tk_popup  .wiring.csvImport.context $X $Y
}
