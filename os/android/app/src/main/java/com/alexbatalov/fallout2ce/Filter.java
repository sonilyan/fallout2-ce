package com.alexbatalov.fallout2ce;

import java.io.File;
import java.io.FileFilter;

public class Filter implements FileFilter {
    @Override
    public boolean accept(File file) {
        if(file.isHidden())
            return false;
        if(file.isDirectory())
            return true;
        return false;
    }
}
