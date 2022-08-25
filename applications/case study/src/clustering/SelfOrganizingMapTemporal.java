package clustering;

import math.Matrix;
import math.Vector;

public class SelfOrganizingMapTemporal extends SelfOrganizingMap {

	public final static double defaultTemporalAlpha = 0.8;
	public final static double defaultTemporalBeta = 0.2;
	
	private double temporalAlpha, temporalBeta;	
	
	public SelfOrganizingMapTemporal(int width) {
		super(width);
		this.temporalAlpha = defaultTemporalAlpha;
		this.temporalBeta = defaultTemporalBeta;
	}

	public SelfOrganizingMapTemporal(int width, double startLearningRate,
			double temporalAlpha, double temporalBeta) {
		super(width, startLearningRate);
		this.temporalAlpha = temporalAlpha;
		this.temporalBeta = temporalBeta;
	}

	@Override public void feedData(Matrix data, int numClusters,
			double threshold, int maxIter) {
		curData = data;
		int dim = data.height();
		int num = data.width();

		int height = numClusters / width;
		double mapRadius = Math.max(width, height) / 2.0;
		double timeConstant = maxIter / Math.log(mapRadius);
		
		// initialize the model vector of each node (center) with small random
		// values
		nodes = new Vector[numClusters];
		Vector[] context = new Vector[numClusters];
		for (int i = 0; i < numClusters; i++) {
			nodes[i] = new Vector(dim);
			for (int j = 0; j < dim; j++)
				nodes[i].set(j, Math.random());
			context[i] = new Vector(2);
		}
		
		Vector prevBmu = new Vector(2);
		
		for (int i = 0; i < maxIter; i++) {  // threshold is ignored
			Vector v = data.getColumn(i % num);

			// find Best Matching Unit
			double minDist = Double.POSITIVE_INFINITY;
			int minDistIdx = 0;
			for (int j = 0; j < numClusters; j++) {
				double d = temporalAlpha * v.euclideanDistSqr(nodes[j]);
				double x = prevBmu.get(0) - context[j].get(0);
				double y = prevBmu.get(1) - context[j].get(1);
				d += temporalBeta * ((x * x) + (y * y));
				if (d < minDist) {
					minDist = d;
					minDistIdx = j;
				}
			}

			int curBmuX = minDistIdx % width;
			int curBmuY = minDistIdx / width;

			// determine local neighborhood of BMU
			double r = mapRadius * Math.exp(-((double) i / timeConstant));
			r *= r;  // we use squared euclidean distance
			for (int j = 0; j < numClusters; j++) {
				int x = curBmuX - (j % width);
				int y = curBmuY - (j / width);
				int d = (x * x) + (y * y);

				if (d < r) {  // is within radius r
					// adjust model vector
					double learningRate = startLearningRate *
						Math.exp(-((double) i / maxIter));
					learningRate *= Math.exp(-d / (2.0 * r));
					nodes[j].add(Vector.mult(learningRate,
							Vector.sub(v, nodes[j])));

					// update context
					context[j].add(Vector.mult(learningRate,
							Vector.sub(prevBmu, context[j])));
				}
			}
			
			prevBmu.set(0, curBmuX);
			prevBmu.set(1, curBmuY);
		}
	}

}
