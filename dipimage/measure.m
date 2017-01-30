%MEASURE   Do measurements on objects in an image
%
% SYNOPSIS:
%  msr = measure(object,gray,measurmentIDs,objectIDs,connectivity)
%
% PARAMETERS:
%  object: binary or labelled image holding the objects.
%  gray: (original) gray value image of object_in. It is needed for
%        several types of measurements. Otherwise you can use [].
%  featureIDs: measurements to be performed, either a single string
%              or a cell array with strings (e.g.: {'Size','Perimeter'} ).
%  objectIDs: labels of objects to be measured. Use [] to measure all
%             objects
%  connectivity: defines which pixels are considered neighbours. Match
%                this value to the one used to label object_in.
%
% DEFAULTS:
%  gray_in = [];
%  featureIDs = 'size'
%  objectIDs = []
%  connectivity = ndims(object_in)
%
% RETURNS:
%  msr: a dip_measurement object containing the results.
%
% EXAMPLE:
%  img = readim('cermet')
%  msr = measure(img<100, img, ({'size', 'perimeter','mean'}))
%
% NOTE:
%  MEASURE HELP prints a list of all measurement features available in this
%  function.
%
% NOTE:
%  Several measures use the boundary chain code (i.e. 'Feret', 'Perimeter',
%  'BendingEnergy', 'P2A' and 'PodczeckShapes'). These measures will fail
%  if the object is not compact (one chain code must represent the whole
%  object). If the object is not compact under the connectivity chosen, only
%  one part of the object will be measured.
%
% NOTE:
%  See the user guide for the definition of connectivity in DIPimage.
