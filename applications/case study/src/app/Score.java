package app;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.Vector;

public class Score {

	public static void readClusterIndices(String fileName, Vector<Integer> v)
			throws IOException {
		File dataFile = new File(fileName);
		BufferedReader r = new BufferedReader(new FileReader(dataFile));
		String line;
		while ((line = r.readLine()) != null) {
			String parts[] = line.split(";");
			if (parts.length > 1)
				v.add(parts[1].charAt(0) - 'A');
		}		
	}
	
	private final static int permutations[][] = {
		{ 0, 1, 2 },
		{ 0, 2, 1 },
		{ 1, 0, 2 },
		{ 1, 2, 0 },
		{ 2, 0, 1 },
		{ 2, 1, 0 }
	};
	
	private static Vector<Integer> permutate(Vector<Integer> source,
			int permutation[]) {
		Vector<Integer> dest = new Vector<Integer>(source.size());
		for (int i = 0; i < source.size(); i++)
			dest.add(permutation[source.get(i)]);
		return dest;
	}
	
	public static void main(String[] args) {
		if (args.length < 2) {
			System.err.println("usage: Score clustering reference");
			System.exit(1);
		}
		
		Vector<Integer> clusters = new Vector<Integer>();
		Vector<Integer> refClusters = new Vector<Integer>();

		try {
			readClusterIndices(args[0], clusters);
			readClusterIndices(args[1], refClusters);
		}
		catch (IOException ex) {
			System.err.println(ex.getMessage());
			System.exit(1);
		}

		int bestPermutationIdx = 0;
		double bestPermutationScore = 0.0;
		for (int i = 0; i < permutations.length; i++) {
			System.out.println("permutation " + Integer.toString(i));
			Vector<Integer> permClusters = permutate(clusters, permutations[i]);
			
			double avgFScore = 0.0;
			double accuracy = 0.0;
			for (int j = 0; j < 3; j++) {
				int found = 0;
				int relevant = 0;
				int foundRelevant = 0;

				for (int k = 0; k < permClusters.size(); k++) {
					if (permClusters.get(k) == j)
						found++;
					if (refClusters.get(k) == j)
						relevant++;
					if ((permClusters.get(k) == j) && (refClusters.get(k) == j))
						foundRelevant++;
				}
				accuracy += foundRelevant;
				
				double fScore = 0.0, precision = 0.0, recall = 0.0;
				recall = (double) foundRelevant / relevant;
				precision = (double) foundRelevant / found;
				if ((precision + recall) > 0)
					fScore = (2.0 * precision * recall) / (precision + recall);
				
				System.out.println("recall for cluster " + Integer.toString(j) + " = " + Double.toString(recall));
				System.out.println("precision for cluster " + Integer.toString(j) + " = " + Double.toString(precision));
				System.out.println("F-score for cluster " + Integer.toString(j) + " = " + Double.toString(fScore));
				
				avgFScore += fScore;
			}
			avgFScore /= 3.0;
			accuracy /= permClusters.size();
			
			System.out.println("average F-score = " + Double.toString(avgFScore));
			System.out.println("accuracy = " + Double.toString(accuracy));

			if (avgFScore > bestPermutationScore) {
				bestPermutationScore = avgFScore;
				bestPermutationIdx = i;
			}
			
			System.out.println();
		}
		
		System.out.println("best permutation = " +
				Integer.toString(bestPermutationIdx));
	}

}
