package app;

import java.io.File;
import java.io.FilenameFilter;
import java.util.HashMap;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import data.DataRecord;
import math.Matrix;

public class XmlBlockReader {

	private class FileMaskFilter implements FilenameFilter {

		private final String reservedChars = ".\\$^";
		private String regex;
		
		public FileMaskFilter(String mask) {
			regex = new String();
			// convert mask to regex
			for (int i = 0; i < mask.length(); i++) {
				char c = mask.charAt(i);
				if (c == '*')
					regex += ".*";
				else {
					if (reservedChars.indexOf(c) != -1)
						regex += "\\";
					regex += c;
				}
			}
		}
		
		public boolean accept(File dir, String fileName) {
			return fileName.matches(regex);
		}

	}


	public static final int dimX = 0;
	public static final int dimY = 1;
	public static final int dimWidth = 2;
	public static final int dimHeight = 3;
	public static final int dimFirstAttr = 4;
	
	private int numDimensions;
	private HashMap<String, Integer> attrIndexMap;
	private Matrix data, normalizedData;
	
	public XmlBlockReader(String fileMask, String attributes[]) {
		numDimensions = dimFirstAttr;
		attrIndexMap = new HashMap<String, Integer>();
		for (int i = 0; i < attributes.length; i++) {
			attrIndexMap.put(attributes[i], numDimensions);
			if (attributes[i].endsWith("Color"))
				// color values will be split into 3 components
				numDimensions += 3;
			else
				numDimensions += 1;
		}

		// find files matching the specified mask
		int lastPathSep = fileMask.lastIndexOf(File.separatorChar);
		String filePath, fileSpec;
		if (lastPathSep >= 0) {
			filePath = fileMask.substring(0, lastPathSep + 1); 
			fileSpec = fileMask.substring(lastPathSep + 1);
		}
		else {
			filePath = System.getProperty("user.dir");  // current directory
			fileSpec = fileMask;
		}
		
		File currentDir = new File(filePath);
		File[] matchingFiles =
			currentDir.listFiles(new FileMaskFilter(fileSpec));
		if (matchingFiles.length == 0)
			throw new RuntimeException("no input files found");
		
		// read block data into matrix
		for (int i = 0; i < matchingFiles.length; i++)
			readXmlFile(matchingFiles[i]);
	}

	private void readXmlFile(File f) {
		try {
			DocumentBuilderFactory factory =
				DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document doc = builder.parse(f);
            
            NodeList blocks = doc.getElementsByTagName("Block");
            int numBlocks = blocks.getLength();
            double docData[][] = new double[numDimensions][];
            for (int i = 0; i < numDimensions; i++)
            	docData[i] = new double[numBlocks];
            for (int i = 0; i < numBlocks; i++) {
            	Element block = (Element) blocks.item(i);
            	
            	// extract position and size
            	docData[dimX][i] = Integer.parseInt(block.getAttribute("left"));
            	docData[dimY][i] = Integer.parseInt(block.getAttribute("top"));
            	docData[dimWidth][i] = Integer.parseInt(
            			block.getAttribute("right")) - docData[dimX][i];
            	docData[dimHeight][i] = Integer.parseInt(
            			block.getAttribute("bottom")) - docData[dimY][i];
            	
            	// extract requested data
            	NodeList extAttrs = block.getElementsByTagName("Attribute");
            	for (int j = 0; j < extAttrs.getLength(); j++) {
            		Element extAttr = (Element) extAttrs.item(j);
            		String extAttrName = extAttr.getAttribute("name");
            		Integer idx = attrIndexMap.get(extAttrName);
            		if (idx != null) {
            			String extAttrValue = extAttr.getAttribute("value");
            			if (extAttrName.endsWith("Color")) {
            				docData[idx][i] = Integer.parseInt(
            						extAttrValue.substring(1, 3), 16);
            				docData[idx + 1][i] = Integer.parseInt(
            						extAttrValue.substring(3, 5), 16);
            				docData[idx + 2][i] = Integer.parseInt(
            						extAttrValue.substring(5, 7), 16);            				
            			}
            			else
            				docData[idx][i] = Double.parseDouble(extAttrValue);
            		}
            	}
            }

            if (data == null)
            	data = new Matrix(docData);
            else
            	data.appendRight(docData);
		}
		catch (Exception ex) {
			throw new RuntimeException(ex.getMessage());
		}
	}
	
	public Matrix getData() {
		return data;
	}
	
	public Matrix getNormalizedData() {
		if (normalizedData == null) {
			normalizedData = new Matrix(data);
			
			// assume vertical resolution of 768 pixels
			for (int i = 0; i < normalizedData.width(); i++)
				normalizedData.set(dimY, i, normalizedData.get(dimY, i) % 768);
			
			DataRecord r = new DataRecord(normalizedData);
			r.normalize();
		}
		return normalizedData;
	}
	
}
