package clustering;

import math.Matrix;
import math.Vector;

public class KMeansPlusPlus extends KMeans {

	private double minDist(Vector x, Vector centers[]) {
		double min = Double.POSITIVE_INFINITY;

		int i = 0;
		while (centers[i] != null) {
			double d = x.euclideanDistSqr(centers[i]);
			if (d < min)
				min = d;
			i++;
		}

		return min;
	}
	
	@Override protected void initCenters(Vector centers[], Matrix data) {
		// choose first center at random
		int idx = (int) Math.round(Math.random() * (data.width() - 1));
		centers[0] = data.getColumn(idx);
		
		for (int i = 1; i < centers.length; i++) {
			Vector v;

			// compute sum of squared minimum distances of all points
			double pDiv = 0.0;
			for (int j = 0; j < data.width(); j++) {
				v = data.getColumn(j);
				pDiv += minDist(v, centers); // minDist returns squared distance
			}
			
			while (centers[i] == null) {
				idx = (int) Math.round(Math.random() * (data.width() - 1));
				v = data.getColumn(idx);
				double p = minDist(v, centers) / pDiv;
				if (Math.random() < p)
					centers[i] = new Vector(v);
			}
		}
	}
	
}
