# HTMLex

A plugin ("Browser Helper Object", BHO) for Internet Explorer 6.0. Walks the DOM
tree to identify spans of text, that is, sequences of characters of maximal
length that are homogeneous in terms of their style (same font face, size,
color, etc.). The rectangular bounding boxes of these spans do not overlap. For
each of these spans, the plugin records the position and size of the bounding
box, as well as all relevant stylistic attributes.

This data is useful for various kinds of machine learning tasks. The case study
in directory `applications/case study` demonstrates how unsupervised techniques
alone can successfully distinguish different structural elements of a news
website (navigation, headline, text body) and different types of websites (blogs
vs. discussion boards).

Directory `applications/2D-CRF` contains an implementation of the 2-dimensional
CRF described by Zhu et al. ("2D Conditional Random Fields for Web Information
Extraction", Proceedings of the 22nd International Conference on Machine
Learning, 2005), based on the [CRF library](http://crf.sourceforge.net) written
by Sarawagi. The intention was to explore the utility of the website layout
data extracted by HTMLex for supervised classification tasks, but that project
was abandoned. The implementation is accompanied by a simple test case using
synthetic data (class `iitb.TwoDimensionalClassifier.TwoDimensionalClassifier`),
but has not been tested otherwise.
