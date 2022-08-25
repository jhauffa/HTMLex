package app;

import java.awt.Color;
import java.awt.Font;
import java.io.OutputStreamWriter;
import java.util.ArrayList;

import data.DataRecord;
import math.Matrix;
import math.Vector;

import org.apache.batik.dom.GenericDOMImplementation;
import org.apache.batik.svggen.SVGGraphics2D;
import org.w3c.dom.DOMImplementation;
import org.w3c.dom.Document;

import clustering.FuzzyCMeans;
import clustering.IClusteringAlgorithm;
import clustering.KMeans;
import clustering.KMeansPlusPlus;
import clustering.SelfOrganizingMap;
import clustering.SelfOrganizingMapTemporal;

public class BlockClustering {

	private static final String usage = 
		"usage: BlockClustering [-aalgo][-nx][-ix][-tx][-rx][-oout] file\n" +
		"  -a  specifies the algorithm to be used; possible values:\n" +
		"        cmeans, kmeans, kmeans++, SOM, SOMSD (default = cmeans)\n" +
		"      some algorithms accept optional parameters:\n" +
		"        cmeans,m\n" +
		"        SOM,width,startLearningRate\n" + 
		"        SOMSD,width,startLearningRate,alpha,beta\n" +
		"  -n  x specifies number of clusters (default = 2)\n" +
		"  -i  perform x iterations (default = 10000)\n" +
		"  -t  threshold (interpretation depends on chosen algorithm,\n" +
		"      default = 0.01)\n" +
		"  -r  algorithm for reordering the blocks when using SOMSD\n" +
		"      (default = 0)\n" +
		"        0 = top to bottom, then left to right\n" + 
		"        1 = left to right, then top to bottom\n" +
		"        2 = top to bottom, then distance to vertical middle axis\n" +
		"        3 = left top to bottom right, then distance to diagonal\n" +
		"        other value = do not reorder blocks\n" +
		"  -o  output (default = blocks)\n" +
		"        blocks     blocks, fill color indicates cluster\n" +
		"        clusters   cluster centers\n" +
		"        umatrix    visualization of the SOM's U-matrix\n" +
		"        text       number of block;cluster\n" +
		"  -c  show cluster centers instead of clustered data\n" +
		"  file can be a file name or a mask (e.g. *.xml)";
	
	private static void exitWithError(String msg) {
		if (msg.length() > 0)
			System.err.println(msg);
		System.err.println(usage);
		System.exit(1);
	}
	
	private static String parseAlgoString(String s, ArrayList<Double> params) {
		String parts[] = s.split(",");
		for (int i = 1; i < parts.length; i++) {
			try {
				params.add(Double.parseDouble(parts[i]));
			}
			catch (NumberFormatException ex) {
				// ignore
			}
		}
		return parts[0];
	}

	private static final String attributes[] = {
		"realBackgroundColor", "realTextColor", "realFontSize",	"realFontWeight"
	};
	
	private static final int dimBgColorR = XmlBlockReader.dimFirstAttr;
	private static final int dimBgColorG = XmlBlockReader.dimFirstAttr + 1;
	private static final int dimBgColorB = XmlBlockReader.dimFirstAttr + 2;
	private static final int dimTextColorR = XmlBlockReader.dimFirstAttr + 3;
	private static final int dimTextColorG = XmlBlockReader.dimFirstAttr + 4;
	private static final int dimTextColorB = XmlBlockReader.dimFirstAttr + 5;
	private static final int dimFontSize = XmlBlockReader.dimFirstAttr + 6;
	private static final int dimFontWeight = XmlBlockReader.dimFirstAttr + 7;
	
	private static void drawBlock(SVGGraphics2D canvas, Vector block,
			Color fillColor) {
		// extract block attributes from vector
		int x = (int) block.get(XmlBlockReader.dimX);
		int y = (int) block.get(XmlBlockReader.dimY);
		int w = (int) block.get(XmlBlockReader.dimWidth);
		int h = (int) block.get(XmlBlockReader.dimHeight);
		Color textColor = new Color((int) block.get(dimTextColorR),
		                            (int) block.get(dimTextColorG),
		                            (int) block.get(dimTextColorB));
		Font font;
		int fontSize = (int) block.get(dimFontSize);
		if (((int) block.get(dimFontWeight)) >= 700)
			font = new Font(null, Font.BOLD, fontSize);
		else
			font = new Font(null, Font.PLAIN, fontSize);

		// draw rectangle
		canvas.setColor(Color.black);
		canvas.drawRect(x, y, w, h);
		canvas.setColor(fillColor);
		canvas.fillRect(x, y, w, h);
		
		// draw font sample
		canvas.setColor(textColor);
		canvas.setFont(font);
		canvas.drawString("abc", x + 1, y + fontSize + 1);
	}
	
	public static void main(String[] args) {
		// parse command line arguments
		String fileMask = null, algoName = "cmeans", outputType = "blocks";
		ArrayList<Double> algoParams = new ArrayList<Double>();
		int numClusters = 2;
		int numIter = 10000;
		double threshold = 0.01;
		int reorderAlgo = 0;
		
		for (int i = 0; i < args.length; i++) {
			if ((args[i].length() > 1) && (args[i].charAt(0) == '-')) {
				char opt = args[i].charAt(1);
				switch (opt) {
				case 'a' :
					algoName = parseAlgoString(args[i].substring(2),
							algoParams);
					break;
				case 'n' :
					try {
						numClusters = Integer.parseInt(args[i].substring(2));
					}
					catch (NumberFormatException ex) {
						// use default value
					}
					break;
				case 'i' :
					try {
						numIter = Integer.parseInt(args[i].substring(2));
					}
					catch (NumberFormatException ex) {
						// use default value
					}
					break;
				case 't' :
					try {
						threshold = Double.parseDouble(args[i].substring(2));
					}
					catch (NumberFormatException ex) {
						// use default value
					}
					break;
				case 'r' :
					try {
						reorderAlgo = Integer.parseInt(args[i].substring(2));
					}
					catch (NumberFormatException ex) {
						// use default value
					}
					break;
				case 'o' :
					outputType = args[i].substring(2);
					break;
				default :
					System.err.println("ignoring invalid option " + opt);
				}
			}
			else
				fileMask = args[i];
		}
		
		if (fileMask == null)
			exitWithError("no input file specified");

		// read XML files matching the specified mask
		Matrix blocks = null;
		Matrix blocksNorm = null;
		try {
			XmlBlockReader blockReader =
				new XmlBlockReader(fileMask, attributes);
			blocks = blockReader.getData();
			blocksNorm = blockReader.getNormalizedData();
		}
		catch (RuntimeException ex) {
			System.err.println("Error parsing XML input: " + ex.getMessage());
			System.exit(1);
		}
		
		// set parameters for clustering
		IClusteringAlgorithm algo = null;
		if (algoName.equals("cmeans")) {
			double m = FuzzyCMeans.defaultM;
			if (algoParams.size() > 0)
				m = algoParams.get(0);
			algo = new FuzzyCMeans(m);
		}
		else if (algoName.equals("kmeans"))
			algo = new KMeans();
		else if (algoName.equals("kmeans++"))
			algo = new KMeansPlusPlus();
		else if (algoName.startsWith("SOM")) {
			int width = (int) Math.sqrt(numClusters);
			if (algoParams.size() > 0)
				width = (int) Math.round(algoParams.get(0));

			double startLearningRate =
				SelfOrganizingMap.defaultStartLearningRate;
			if (algoParams.size() > 1)
				startLearningRate = algoParams.get(1);
			
			if (algoName.equals("SOMSD")) {
				if (reorderAlgo < 4) {
					NormalizedBlockCompare cmp =
						new NormalizedBlockCompare(reorderAlgo);
					int permutation[] = new int[blocks.width()];
					DataRecord rBlocksNorm = new DataRecord(blocksNorm);
					rBlocksNorm.sort(cmp, permutation);
					DataRecord rBlocks = new DataRecord(blocks);
					rBlocks.permutate(permutation);
				}
				
				double alpha = SelfOrganizingMapTemporal.defaultTemporalAlpha;
				if (algoParams.size() > 2)
					alpha = algoParams.get(2);
				
				double beta = SelfOrganizingMapTemporal.defaultTemporalBeta;
				if (algoParams.size() > 3)
					alpha = algoParams.get(3);
				
				algo = new SelfOrganizingMapTemporal(width, startLearningRate,
						alpha, beta);
			}
			else
				algo = new SelfOrganizingMap(width, startLearningRate);
		}
		else
			exitWithError("unknown algorithm: " + algoName);

		// perform clustering on normalized block data
		algo.feedData(blocksNorm, numClusters, threshold, numIter);
		
		// render result as SVG
		DOMImplementation domImpl =
			GenericDOMImplementation.getDOMImplementation();
		Document svgDoc =
			domImpl.createDocument("http://www.w3.org/2000/svg", "svg", null);
		SVGGraphics2D svgCanvas = new SVGGraphics2D(svgDoc);
		
		Vector centers[] = algo.getCenters();
		if (outputType.equals("clusters")) {
			for (int i = 0; i < centers.length; i++) {
				DataRecord r = new DataRecord(blocks);
				r.expand(centers[i]);
				
				// fill color == background color
				Color fillColor = new Color((int) centers[i].get(dimBgColorR),
                                            (int) centers[i].get(dimBgColorG),
                                            (int) centers[i].get(dimBgColorB));
				drawBlock(svgCanvas, centers[i], fillColor);
			}
		}
		else if (outputType.equals("blocks")) {
			Matrix assoc = algo.getAssociationMatrix();
			for (int i = 0; i < assoc.height(); i++) {
				// find cluster with highest probability of association
				int clustIdx = 0;
				double clustProb = 0.0;
				for (int j = 0; j < assoc.width(); j++) {
					double p = assoc.get(i, j);
					if (p > clustProb) {
						clustIdx = j;
						clustProb = p;
					}
				}
				
				// Scale probability of association so that it is 0 if all
				// clusters are equally probable.
				clustProb = (clustProb * (double) numClusters - 1.0) /
				            ((double) numClusters - 1.0);
				Color fillColor = Color.getHSBColor(
					(float) clustIdx / numClusters, (float) clustProb, 1.0f);
				drawBlock(svgCanvas, blocks.getColumn(i), fillColor);
			}
		}
		else if (outputType.equals("umatrix")) {
			if (!algoName.startsWith("SOM"))
				exitWithError("output type umatrix, algo != SOM");

			SelfOrganizingMap som = (SelfOrganizingMap) algo;
			Matrix u = som.getUMatrix();
			u.normalize();

			for (int i = 0; i < u.height(); i++) {
				for (int j = 0; j < u.width(); j++) {
					Color fillColor = Color.getHSBColor(0.2f,
							1.0f, (float) u.get(i, j));
					svgCanvas.setColor(fillColor);
					svgCanvas.fillRect((j + 1) * 18, (i + 1) * 18, 15, 15);
				}
			}
		}
		else if (outputType.equals("text")) {
			Matrix assoc = algo.getAssociationMatrix();
			for (int i = 0; i < assoc.height(); i++) {
				// find cluster with highest probability of association
				int clustIdx = 0;
				double clustProb = 0.0;
				for (int j = 0; j < assoc.width(); j++) {
					double p = assoc.get(i, j);
					if (p > clustProb) {
						clustIdx = j;
						clustProb = p;
					}
				}
				
				System.out.println(Integer.toString(i) + ";" +
						((char) ('A' + clustIdx)));
			}

			System.exit(0);  // hack; do not output empty SVG
		}
		else
			exitWithError("invalid output type: " + outputType);
		
		try {
			OutputStreamWriter out =
				new OutputStreamWriter(System.out, "UTF-8");
			svgCanvas.stream(out, true);
		}
		catch (Exception ex) {
			System.out.println(ex.getMessage());
		}
	}

}
