/* 
YOUR NAME HERE
NAME: Tan Wei Xiang, Calvin
MATRICULATION NUMBER: A0217529Y
*/

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.File;
import java.io.IOException;
import java.util.StringTokenizer;
import java.util.HashSet;
import java.util.HashMap;
import java.io.InputStreamReader;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.KeyValueTextInputFormat;
import org.apache.hadoop.mapreduce.lib.input.MultipleInputs;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;


/*
 * Idea: Just use the WordCount.java result to another MapReduce task
 * 
 * 1. Do WordCount on the 2 files and store the outputs in separate folder
 * 2. Feed the 2 outputs into the same mapper for formatting
 * 3. Use the same reducer in part0 as the combiner, output of combiner will be
 *    <word, List<counts>>
 * 4. If the List of counts has 2 values, means its a common word 
 * 5. Find the smaller number of times it appears between the two files.
 * 5. Use a linked list map and custom comparator to output the common words and count
 *    sorted order of occurrence frequency and lexicographic order.
 * 
 */

public class TopkCommonWords {

    // Identical to the mapper for assign0 except we need remove the stop words
    // and find the words > 4 characters
    public static class TokenizerMapper
            extends Mapper<Object, Text, Text, IntWritable> {

        private final static IntWritable one = new IntWritable(1);
        private Text word = new Text();

        // Read from stopWordsFile line by line and output it to a HashMap
        private HashSet<String> stopWordsSet = new HashSet<>();

        // read in the stop words file from conf once
        @Override
        protected void setup(Context context) throws IOException, InterruptedException {
            Configuration conf = context.getConfiguration();
            String path = conf.get("stopWords");
            File file = new File(path);
            BufferedReader br = new BufferedReader(new FileReader(file));
            String line;
            while ((line = br.readLine()) != null) {
                stopWordsSet.add(line);
            }
            br.close();
            
        }

        public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
            StringTokenizer itr = new StringTokenizer(value.toString());
            while (itr.hasMoreTokens()) {

                // Remove all the special characters
                String token = itr.nextToken();
                token = token.replaceAll("\n", "");
                token = token.replaceAll("\t", "");
                token = token.replaceAll("\r", "");
                token = token.replaceAll("\f", "");

                // Check if word is in the stop words list
                // If it is, then skip
                if (stopWordsSet.contains(token)) {
                    continue;
                }

                // Check if word > 4 characters
                // If it isn't, then skip
                if (token.length() <= 4) {
                    continue;
                }
                word.set(token);
                context.write(word, one);
            }
        }
    }

    // Same as the reducer for assign0
    public static class IntSumReducer
            extends Reducer<Text, IntWritable, Text, IntWritable> {
        private IntWritable result = new IntWritable();

        public void reduce(Text key, Iterable<IntWritable> values,
                Context context) throws IOException, InterruptedException {
            int sum = 0;
            for (IntWritable val : values) {
                sum += val.get();
            }
            result.set(sum);
            context.write(key, result);
        }
    }

    // Reads in the word count for a file and outputs the correct format for the
    // reducer
    // Takes in (k1, v1) and outputs List(k2, v2)
    public static class FormattingMapper1 extends Mapper<Object, Text, Text, IntWritable> {
        private IntWritable counts = new IntWritable();
        private Text word = new Text();

        public void map(Object key, Text value, Context context)
                throws IOException, InterruptedException {
                
            String[] wordCount = (value.toString()).split("\t"); // by default the results from original word count are \t delimited
            word.set(wordCount[0]);
            counts.set(Integer.parseInt(wordCount[1]));
            context.write(word, counts);
        }

    }

    
    // reducer that takes in the two files and outputs the top k common words
    // in sorted order of occurrence frequency and lexicographic order
    public static class TopKReducer 
            extends Reducer<Text, IntWritable, IntWritable, Text> {

        private Text word = new Text();
        private IntWritable smallerCount = new IntWritable();
        private HashMap<String, Integer> unsortedCommonWords;

        
        // Sorts the HashMap by count in decreasing order then by word in lexicographic
        private LinkedHashMap<String, Integer> sortHashMapByValues(HashMap<String, Integer> hm) {
            List<Map.Entry<String, Integer>> list = new LinkedList<Map.Entry<String, Integer>>(hm.entrySet());

            Collections.sort(list, new Comparator<Map.Entry<String, Integer>>() {
                public int compare(Map.Entry<String, Integer> entryOne, Map.Entry<String, Integer> entryTwo) {
                    int result = entryTwo.getValue().compareTo(entryOne.getValue());
                    if (result == 0) {
                        return entryOne.getKey().compareTo(entryTwo.getKey());
                    }
                    return result;
                }
            });

            LinkedHashMap<String, Integer> sortedHashMap = new LinkedHashMap<String, Integer>();
            for (Map.Entry<String, Integer> entry : list) {
                sortedHashMap.put(entry.getKey(), entry.getValue());
            }

            return sortedHashMap;
        }

        // Init a hash map once
        @Override
        protected void setup(Context context) throws InterruptedException, IOException {
          super.setup(context);
          unsortedCommonWords = new HashMap<String, Integer>();
        }

        public void reduce(Text key, Iterable<IntWritable> values,
                Context context) throws IOException, InterruptedException {
            int numTimesAppears = 0;
            int countsOne = 0;
            int countsTwo = 0;
            int smallerCountValue = 0;

            // If the word appears twice, means it is common word
            // Then find the smaller number of times it appears between the two files
            for (IntWritable value : values) { 
                numTimesAppears++;
                if (numTimesAppears == 1) {
                    countsOne = value.get();
                } else {
                    countsTwo = value.get();
                }

                if (numTimesAppears == 2) {
                    smallerCountValue = Math.min(countsOne, countsTwo);
                    unsortedCommonWords.put(key.toString(), smallerCountValue);
                }
            
            }

        }
        
        // to be run once at the end
        @Override
        protected void cleanup(Context context) throws InterruptedException, IOException {
           super.cleanup(context);

            Configuration conf = context.getConfiguration();

            // Sort the HashMap by count in decreasing order then by word in lexicographic order
            LinkedHashMap<String, Integer> sortedMap = sortHashMapByValues(unsortedCommonWords);

            int numOutputted = 0;
            int kValue = context.getConfiguration().getInt("kValue", 10); // default 10
            for (Map.Entry<String, Integer> entry : sortedMap.entrySet()) {
                if (numOutputted < kValue) {
                    word.set(entry.getKey());
                    smallerCount.set(entry.getValue());
                    context.write(smallerCount, word);
                    numOutputted++;
                } else {
                  break;
                }
            }
          
        }
    }


    public static void main(String[] args) throws Exception {

        // Read in all the args
        Path inFileOne = new Path(args[0]);
        Path inFileTwo = new Path(args[1]);
        String stopWordsFile = args[2];
        Path outputDir = new Path(args[3]);
        int kValue = Integer.parseInt(args[4]);

        Configuration conf = new Configuration();
        conf.setInt("kValue", kValue);
        conf.set("stopWords", stopWordsFile);

        FileSystem fs = FileSystem.get(conf);

        // Paths to store the intermediate results
        Path interDirPath = new Path("/home/course/cs4225/cs4225_assign/temp/assign1_inter/A0217529Y");
        Path fileOneInterDirPath = new Path("/home/course/cs4225/cs4225_assign/temp/assign1_inter/A0217529Y/file1");
        Path fileTwoInterDirPath = new Path("/home/course/cs4225/cs4225_assign/temp/assign1_inter/A0217529Y/file2");
        
        if (fs.exists(fileOneInterDirPath)) {
            fs.delete(fileOneInterDirPath);
        }
        if (fs.exists(fileTwoInterDirPath)) {
            fs.delete(fileTwoInterDirPath);
        }
        if (fs.exists(interDirPath)) {
            fs.delete(interDirPath);
        }

        // Get word count for inFileOne
        Job job1 = Job.getInstance(conf, "inFileOne");
        job1.setJarByClass(TopkCommonWords.class);
        job1.setMapperClass(TokenizerMapper.class);
        job1.setCombinerClass(IntSumReducer.class);
        job1.setReducerClass(IntSumReducer.class);
        job1.setOutputKeyClass(Text.class);
        job1.setOutputValueClass(IntWritable.class);
        FileInputFormat.addInputPath(job1, inFileOne);
        FileOutputFormat.setOutputPath(job1, fileOneInterDirPath);
        job1.waitForCompletion(true);

        // Get word count for inFileTwo
        Job job2 = Job.getInstance(conf, "inFileTwo");
        job2.setJarByClass(TopkCommonWords.class);
        job2.setMapperClass(TokenizerMapper.class);
        job2.setCombinerClass(IntSumReducer.class);
        job2.setReducerClass(IntSumReducer.class);
        job2.setOutputKeyClass(Text.class);
        job2.setOutputValueClass(IntWritable.class);
        FileInputFormat.addInputPath(job2, inFileTwo);
        FileOutputFormat.setOutputPath(job2, fileTwoInterDirPath);
        job2.waitForCompletion(true);

        
        // Combine the two word counts into one file
        Job job3 = Job.getInstance(conf, "combined");
        job3.setJarByClass(TopkCommonWords.class);
        FileInputFormat.addInputPath(job3, fileOneInterDirPath);
        FileInputFormat.addInputPath(job3, fileTwoInterDirPath);
        job3.setMapperClass(FormattingMapper1.class);
        job3.setCombinerClass(IntSumReducer.class);
        job3.setReducerClass(TopKReducer.class);
        job3.setMapOutputKeyClass(Text.class);
        job3.setMapOutputValueClass(IntWritable.class);
        job3.setOutputKeyClass(IntWritable.class);
        job3.setOutputValueClass(Text.class);
        job3.setNumReduceTasks(1);
        FileOutputFormat.setOutputPath(job3, outputDir);

        boolean hasCompleted = job3.waitForCompletion(true);
        fs.delete(fileOneInterDirPath, true);
        fs.delete(fileTwoInterDirPath, true);
        fs.delete(interDirPath, true); // ONLY call this after your last job has completed to delete your intermediate directory
        System.exit(hasCompleted ? 0 : 1);


    }
}



