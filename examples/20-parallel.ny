// This example computes the total number of audio files available in each folder
// provided from the command line.
// Note the presence of the & operator which will dispatch each call into a new job.
// Since the operator += is allowed when creating a variable (which will be initialized
// with the default value), the each construct can be used in combination with += to
// obtain a really concise and elegant syntax for a complicated task.

func howManyAudioFilesAtUrl(url)
{
	switch std.io.type(url) do
	{
		case ntFolder:
		{
			// the given url is a folder
			// retrieving a virtual list of all files in this folder, matching our criteria
			var allfiles = (inode in std.io.directory(url).entries) | inode.type == ntFile
				and inode.name == :regex{ .*\.{mp3,wav,ogg,wma} };

			// retrieving how many files this folder has
			return allfiles.size;
		}
	}
	else
		return 0;
}


public func main(args)
{
	// The magic happens here !
	// Using the value for variable howManyFiles triggers a synchronization,
	// i.e. the statement will not run until all jobs are finished and the final value computed
	var howManyFiles += & howManyAudioFilesAtUrl((each in args));

	console << "total number of files : " << howManyFiles << “\n”;
}
