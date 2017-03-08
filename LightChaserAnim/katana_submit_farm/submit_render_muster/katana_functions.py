import re


def convert_frames_string_to_frame_list(frames_string):
	frame_clips = frames_string.split(' ')
	frame_list = []
	for c in frame_clips:
		if re.match('\d{1,4}$', c) != None: #or (re.match('\d{1,4}-\d{1,4}$', c) != None and int(c.split('-')[1]) >= int(c.split('-')[0])):
			print c
			frame_list.append(c)

		elif re.match('\d{1,4}-\d{1,4}$', c) != None and int(c.split('-')[1]) >= int(c.split('-')[0]):
			print c
			for i in range(int(c.split('-')[0]), int(c.split('-')[1])+1):
				frame_list.append(str(i))

		else:
			print 'pattern not match'
			return None

	return frame_list